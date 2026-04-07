/* 
 * File:   sflow.c
 * Author: alex
 * 
 * Created on March 29, 2026, 3:32 PM
 * 
 * Hintertux, Austria.
 */

#include "sflow.h"
#include "vif2.h"
#include "worker.h"
#include "ctrl_plane.h"
#include "utils.h"
#include "cli_server.h"
#include "router_conf_parser.h"

/*
 * Collector shadow-slot handoff design
 *
 * Assumptions:
 * - Only one worker timer is allowed to update the shadow collector slot.
 * - sflow_collector_set() runs on the control-plane thread.
 *
 * Update sequence:
 * 1. sflow_collector_set() checks whether a worker update round is already
 *    in progress. If so, it stores SFLOW_WRK_STATE_NONE and waits at
 *    cplane_msu_barrier() before modifying the shadow slot.
 *
 * 2. After that barrier returns, the control plane can safely overwrite
 *    collector[!collector_ind], because the previous worker round is no
 *    longer allowed to touch that slot.
 *
 * 3. The control plane publishes a new update request by release-storing
 *    SFLOW_WRK_STATE_RUN.
 *
 * 4. The worker timer callback acquires worker_state. Only if it observes
 *    SFLOW_WRK_STATE_RUN does it proceed to initialize the shadow slot.
 *
 * 5. After the worker finishes writing all shadow collector fields, it
 *    release-stores SFLOW_WRK_STATE_COMPLETE.
 *
 * 6. The control-plane timer callback acquires worker_state and publishes
 *    the shadow slot only after observing SFLOW_WRK_STATE_COMPLETE.
 *    It then release-stores collector_ind so workers can start using the
 *    new active slot.
 *
 * 7. Before reusing the old active slot as the next shadow slot, the control
 *    plane waits in cplane_msu_barrier(). This ensures workers have crossed
 *    the MSU quiescent point and no longer use the previous active copy.
 *
 * Result:
 * - The shadow slot is not overwritten while the worker owns it.
 * - A new active slot is not published before worker initialization finishes.
 * - The previous active slot is not reused as shadow storage until workers
 *   have quiesced past the publication point.
 */

void
sflow_worker_(worker_t *worker, sflow_engine_t *engine,
		  const vif2_t *vif, const struct rte_mbuf *m,
		  uint16_t sample_len, uint32_t sampling_rate_be,
		  const unsigned dir)
{
	uint8_t ind;
	uint8_t eth_hdr_len;
	uint16_t padded_sample_len, udp_len, sflow_len;
	uint32_t src_addr_be, lcore_id_be, seq_num_be, total_len;
	struct rte_mbuf *sflow_m;
	void *eth_hdr;
	struct rte_ipv4_hdr *ipv4_hdr;
	struct rte_udp_hdr *udp_hdr;
	struct sflow_hdr *sflow_hdr;
	struct sflow_sample *sflow_sample;
	sflow_collector_t *collector;
	sflow_worker_cnt_t *cnts;
		
	sflow_m = worker_pktmbuf_alloc(worker, DC_WRK);
	if (sflow_m == NULL)
		return;
	
	eth_hdr = rte_pktmbuf_mtod(sflow_m, void *);
	
	ind = atomic_load_acquire(&engine->collector_ind);
	collector = &engine->collector[ind];
	
	eth_hdr_len = collector->eth_hdr_len;
	sample_len = RTE_MIN(sample_len, rte_pktmbuf_data_len(m));
	padded_sample_len = RTE_ALIGN_CEIL(sample_len, 4);
	sflow_len = sizeof(struct sflow_hdr) + sizeof(struct sflow_sample) +
			  padded_sample_len;
	udp_len = sizeof(struct rte_udp_hdr) + sflow_len;
	
	total_len = eth_hdr_len + sizeof(struct rte_ipv4_hdr) + udp_len;
	if (total_len > rte_pktmbuf_data_len(sflow_m) + 
			  rte_pktmbuf_tailroom(sflow_m)) {
		rtr_pktmbuf_free(sflow_m, DC_WRK);
		return;
	}
	
	rte_memcpy(eth_hdr, collector->eth_hdr, eth_hdr_len);
	sflow_m->l2_len = eth_hdr_len;
	
	ipv4_hdr = (struct rte_ipv4_hdr *)((char *)eth_hdr + eth_hdr_len);
	udp_hdr = (struct rte_udp_hdr *)(ipv4_hdr + 1);
	
	cnts = &worker->sflow_worker.counters[dir];
	lcore_id_be = rte_cpu_to_be_32(rte_lcore_id());
	seq_num_be = rte_cpu_to_be_32(++cnts->seq_num);
	
	/* sFlow */
	src_addr_be = atomic_load_relaxed(&collector->src_addr_be);
	sflow_hdr = (struct sflow_hdr *)(udp_hdr + 1);
	sflow_hdr->ver = BR_CONST_BE32(SFLOW_VER);
	sflow_hdr->agent_address_type = BR_CONST_BE32(SFLOW_ADDR_TYPE_IPV4);
	sflow_hdr->agent_address = src_addr_be;
	sflow_hdr->sub_agent_id = lcore_id_be;
	sflow_hdr->seq_num = seq_num_be;
	sflow_hdr->sys_uptime = rte_cpu_to_be_32(RTR_CYCLES_TO_MS(
			  worker->main_loop_iteration_start_time, g_router_params.timer_hz));
		
	sflow_hdr->num_samples = BR_CONST_BE32(1);
	
	sflow_sample = (struct sflow_sample *)sflow_hdr->samples;
		
	sflow_sample->hdr = BR_CONST_BE32(1);
	sflow_sample->len = rte_cpu_to_be_32(
			  sizeof(struct sflow_sample) - FLOW_HDR_LEN +
			  padded_sample_len);
	sflow_sample->seq_num = seq_num_be;
	sflow_sample->source_id = lcore_id_be;
	sflow_sample->sampling_rate = sampling_rate_be;
	sflow_sample->sample_pool = rte_cpu_to_be_32(cnts->total_pkt_cnt);
	sflow_sample->dropped_pkts = 0;
	sflow_sample->intput_iface = 0;
	sflow_sample->output_iface = 0;
	(&sflow_sample->intput_iface)[dir] = rte_cpu_to_be_32(vif->id);
	
	sflow_sample->flow_record = BR_CONST_BE32(1);
		
	sflow_sample->sample_rec.hdr = BR_CONST_BE32(SFLOW_FLOW_SAMPLE);
	sflow_sample->sample_rec.flow_data_len = rte_cpu_to_be_32(
			  sizeof(struct sflow_sample_record) - FLOW_HDR_LEN +
			  padded_sample_len);
	sflow_sample->sample_rec.header_protocol = BR_CONST_BE32(SFLOW_ETH_PROTO);
	sflow_sample->sample_rec.frame_len = rte_cpu_to_be_32(
			  rte_pktmbuf_pkt_len(m));
	sflow_sample->sample_rec.payload_stripped = 0;
	sflow_sample->sample_rec.sample_header_len = rte_cpu_to_be_32(
			  (uint32_t)sample_len);
	rte_memcpy(sflow_sample->sample_rec.sample_header,
			  rte_pktmbuf_mtod(m, const void *), sample_len);
	if (padded_sample_len > sample_len) {
		/* XDR opaque data must be padded to a 4-byte boundary. */
		memset(&sflow_sample->sample_rec.sample_header[sample_len], 0,
				  padded_sample_len - sample_len);
	}
		
	/* IPv4 */
	ipv4_hdr->version_ihl = IPV4_VER(sizeof(struct rte_ipv4_hdr));
	// fixme: add option to overwrite tc
	ipv4_hdr->type_of_service = 0;
	ipv4_hdr->total_length = rte_cpu_to_be_16(sizeof(*ipv4_hdr) + udp_len);
	ipv4_hdr->packet_id = 0;
	ipv4_hdr->fragment_offset = 0;
	ipv4_hdr->time_to_live = 64;
	ipv4_hdr->next_proto_id = IP_PROTO_UDP;
	ipv4_hdr->src_addr = src_addr_be;
	ipv4_hdr->dst_addr = collector->collector_addr_be;
	ipv4_hdr->hdr_checksum = 0;
	
	/* UDP */
	udp_hdr->src_port = BR_CONST_BE16(SFLOW_DEFAULT_SRC_PORT);
	udp_hdr->dst_port = collector->collector_port_be;
	udp_hdr->dgram_len = rte_cpu_to_be_16(udp_len);
		
	/* Offload checksum */
	sflow_m->ol_flags = RTE_MBUF_F_TX_IPV4 | RTE_MBUF_F_TX_IP_CKSUM |
			  RTE_MBUF_F_TX_UDP_CKSUM;
	/* Pre-calculate the Pseudo-Header Checksum for UDP */
	udp_hdr->dgram_cksum = 0;
	udp_hdr->dgram_cksum = rte_ipv4_phdr_cksum(ipv4_hdr, sflow_m->ol_flags);
	
	sflow_m->l3_len = sizeof(*ipv4_hdr);
	sflow_m->data_len = total_len;
	sflow_m->pkt_len = sflow_m->data_len;
	
	/* send to the wire */
	/* send packet to the wire */
	send_single_packet_ipv4_with_ethtype(sflow_m, collector->egress_port_id,
			  collector->egress_mtu, RTE_ETHER_TYPE_IPV4);
}

static void
sflow_collector_update_workers_state(void)
{
	if (FLAG_UP(g_router_params.flags2, ROUTER_PRMS_FL2_SFLOW_GLOBAL_STATE)) {
		/* indicate to workers that sFlow collecting must begin */
		SET_FLAG_UP(g_router_params.flags2, ROUTER_PRMS_FL2_SFLOW_WORKERS);
	}
	else {
		/* indicate to workers that sFlow collecting must be stopped */
		SET_FLAG_DOWN(g_router_params.flags2, ROUTER_PRMS_FL2_SFLOW_WORKERS);
	}
}

static int
sflow_probe_port_lookup(sflow_engine_t *engine, uint8_t port)
{
	for (unsigned i = 0; i < RTE_DIM(engine->enabled_ports); i++)
		if (engine->enabled_ports[i].u64 != SFLOW_UNDEF_PORT &&
				engine->enabled_ports[i].port == port)
			return i;

	return -ERR_NOT_FOUND;
}

int
sflow_probe_port_add(cplane_t *cplane, uint8_t port, 
		  uint16_t rx_sampling_rate, uint16_t tx_sampling_rate,
		  uint16_t sample_len)
{
	int ret;
	sflow_port_t sflow_port;
	sflow_engine_t *engine = cplane->sflow_engine;
		
	if (sample_len < 32 || sample_len > 256)
		return -ERR_INCORRECT_VALUE;
		
	sflow_port.port = port;
	sflow_port.sample_len = sample_len;
	sflow_port.sampling_rates[SFLOW_RX] = rx_sampling_rate;
	sflow_port.sampling_rates[SFLOW_TX] = tx_sampling_rate;
	
	ret = sflow_probe_port_lookup(engine, port);
	if (ret == -ERR_NOT_FOUND) { 
		for (unsigned i = 0; i < RTE_DIM(engine->enabled_ports); i++)
			if (engine->enabled_ports[i].u64 == SFLOW_UNDEF_PORT) {
				atomic_store_relaxed(&engine->enabled_ports[i].u64, sflow_port.u64);
				return OK;
			}
		
		return -ERR_IS_FULL;
	}
			
	atomic_store_relaxed(&engine->enabled_ports[ret].u64, sflow_port.u64);
	return OK;
}

int
sflow_probe_port_del(cplane_t *cplane, uint8_t port)
{
	int ret;
	sflow_engine_t *engine = cplane->sflow_engine;
	
	ret = sflow_probe_port_lookup(engine, port);
	if (ret < 0)
		return ret;
	
	atomic_store_relaxed(&engine->enabled_ports[ret].u64, SFLOW_UNDEF_PORT);
	return OK;
}

static inline sflow_collector_t *
sflow_get_shadow_collector(sflow_engine_t *engine)
{
	uint8_t shadow_col_ind;
	
	shadow_col_ind = !atomic_load_acquire(&engine->collector_ind);
	return &engine->collector[shadow_col_ind];
}

int
sflow_collector_set(cplane_t *cplane, uint32_t addr, uint16_t port)
{
	uint8_t worker_state;
	sflow_collector_t *collector;
	sflow_engine_t *engine = cplane->sflow_engine;
				
	worker_state = atomic_load_relaxed(&engine->worker_state);
	/* 
	 * if the worker is already running or going to run the timer,
	 * let it run it and don't overwrite the data,
	 * wait till the worker job is done.
	 */
	if (worker_state == SFLOW_WRK_STATE_RUN) {
		/* 
		 * update the state, so the worker won't start a new update cycle,
		 * since the current one might not complete the update, for 
		 * example if ARP is not there yet.
		 */
		atomic_store_relaxed(&engine->worker_state, SFLOW_WRK_STATE_NONE);
		cplane_msu_barrier(cplane);
	}
	
	/* at this point either our state store have won or
	 * the timer have completed the job and store SFLOW_WRK_STATE_COMPLETE
	 * state or the state simply was set to SFLOW_WRK_STATE_NONE 
	 * at the first place.
	 */
	worker_state = atomic_load_relaxed(&engine->worker_state);
	assert(worker_state != SFLOW_WRK_STATE_RUN);
	
	/* now it's save to overwrite the shadow collector data */
	collector = sflow_get_shadow_collector(engine);
	collector->collector_port_be = rte_cpu_to_be_16(port);
	collector->collector_addr_be = rte_cpu_to_be_32(addr);
	
	/* 
	 * signal the worker to start updating timer job.
	 */
	atomic_store_release(&engine->worker_state, SFLOW_WRK_STATE_RUN);
		
	/* 
	 * now worker and cplane timers will complete initializing the collector.
	 */
	return OK;
}

static void
sflow_worker_update_collector_timer_cb(struct rte_timer *timer, void *data) 
{
	uint32_t col_addr;
	worker_t *wrk = data;
	arp_entry_t *arp_entry;
	sflow_engine_t *engine = g_cplane->sflow_engine;
	sflow_collector_t *collector;
	vif2_key_t vif_key;
	struct rte_ether_hdr *eth_hdr;
	fib_t *fib;
	fib_next_hop_t *next_hop;
	vif2_t *egress_vif;
	struct rte_mbuf *m_tmp;
	
	(void)timer;
	
	/* run the timer only when cplane data is ready */
	if (atomic_load_acquire(&engine->worker_state) != 
			  SFLOW_WRK_STATE_RUN)
		return;
	
	collector = sflow_get_shadow_collector(engine);
	col_addr = rte_be_to_cpu_32(collector->collector_addr_be);
	fib = msu_reader_get_object(wrk->l3f.reader_msu, L3F_MSU_POI_FIB);
	next_hop = fib_route_lookup(fib->main_route_table, col_addr);
	if (next_hop == NULL)
		return;

	if (FIB_IS_NEXT_HOP_CONNECTED(next_hop)) {
		collector->next_hop_ip = col_addr;
		collector->src_addr_be = rte_cpu_to_be_32(next_hop->src);
		egress_vif = next_hop->next_hop_vif;
	}
	else {
		uint8_t multihop_ind__unused;
		int ret;
		uint32_t nh_src;
		struct rte_ipv4_hdr pseudo_ipv4_hdr;

		pseudo_ipv4_hdr.src_addr = 0;
		pseudo_ipv4_hdr.dst_addr = col_addr;

		ret = fib_next_hop_get(next_hop, &pseudo_ipv4_hdr,
				  &multihop_ind__unused, &collector->next_hop_ip, &nh_src, 
				  &egress_vif);
		if (ret != OK)
			return;

		collector->src_addr_be = rte_cpu_to_be_32(nh_src);
	}

	collector->egress_mtu = egress_vif->mtu;
	collector->egress_port_id = egress_vif->port_id;

	/* 
	 * Ethernet header 
	 */
	collector->eth_hdr_len = egress_vif->l2_hdr_len;
	
	if (collector->eth_hdr_len > sizeof(collector->eth_hdr))
		return;
	
	m_tmp = worker_pktmbuf_alloc(wrk, DC_WRK);
	if (m_tmp == NULL)
		return;
			
	vif2_mbuf_set_vlan_header(egress_vif, m_tmp, RTE_ETHER_TYPE_IPV4);
	rte_memcpy(collector->eth_hdr, rte_pktmbuf_mtod(m_tmp, void *),
			  collector->eth_hdr_len);
	rtr_pktmbuf_free(m_tmp, DC_WRK);
	
	/* zero the rest of the ethernet buf */
	if (sizeof(collector->eth_hdr) > collector->eth_hdr_len)
		memset(&collector->eth_hdr[collector->eth_hdr_len], 0,
				  sizeof(collector->eth_hdr) - collector->eth_hdr_len);
	
	/* ARP */
	vif2_get_key(egress_vif, &vif_key);
	arp_entry = arp_lookup(wrk, &wrk->arp_engine, collector->next_hop_ip,
			  &vif_key, rte_be_to_cpu_32(collector->src_addr_be));
	if (arp_entry == NULL)
		return;
	arp_worker_check_stale(wrk, arp_entry);
	
	eth_hdr = (struct rte_ether_hdr *)collector->eth_hdr;
	rte_ether_addr_copy(vif2_mac_addr(egress_vif), &eth_hdr->src_addr);
	rte_ether_addr_copy(&arp_entry->mac, &eth_hdr->dst_addr);
	
	/* signal the cplane that job is done */
	atomic_store_release(&engine->worker_state,
			  SFLOW_WRK_STATE_COMPLETE);
}

static void
sflow_cplane_update_collector_timer_cb(struct rte_timer *timer, void *data) 
{
	uint8_t shadow_col_ind;
	cplane_t *cplane = data;
	sflow_engine_t *engine = cplane->sflow_engine;
	
	if (FLAG_DOWN(g_router_params.flags2, ROUTER_PRMS_FL2_SFLOW_GLOBAL_STATE)) {
		/* indicate to workers that sFlow collecting must be stopped */
		if (FLAG_UP(g_router_params.flags2, ROUTER_PRMS_FL2_SFLOW_WORKERS))
			SET_FLAG_DOWN(g_router_params.flags2, ROUTER_PRMS_FL2_SFLOW_WORKERS);
		/* no need to update collector since the sFlow is disable globally */
		return;
	}
	
	if (atomic_load_acquire(&engine->worker_state) != 
			  SFLOW_WRK_STATE_COMPLETE)
		return;
	
	/* 
	 * worker has finished it's job, i.e. 
	 * the shadow collector initialization is complete, 
	 * publish it.
	 */
	
	/* 
	 * if collectors copies are equeal, 
	 * then there is no need to change collector index 
	 */
	CTASSERT(sizeof(sflow_collector_t) == 40);
	
	if (memcmp(&engine->collector[0], &engine->collector[1],
			  sizeof(sflow_collector_t)) == 0) {
		/* signal the worker to do another update round */
		sflow_collector_update_workers_state();
		atomic_store_release(&engine->worker_state, SFLOW_WRK_STATE_RUN);
		return;
	}
	
	shadow_col_ind = !atomic_load_relaxed(&engine->collector_ind);
	atomic_store_release(&engine->collector_ind, shadow_col_ind);	

	sflow_collector_update_workers_state();
	/* wait until all workers have started using the new collector copy */
	cplane_msu_barrier(cplane);

	/* 
	 * copy current collector slot to the shadow collector (!ind) slot 
	 * in order to update the it on next timer tics
	 */
	rte_memcpy(&engine->collector[!shadow_col_ind],
			  &engine->collector[shadow_col_ind],
			  sizeof(sflow_collector_t));
	/* signal the worker to do another update round */
	atomic_store_release(&engine->worker_state, SFLOW_WRK_STATE_RUN);
}

int
sflow_engine_init(cplane_t *cplane, sflow_engine_t **out_engine)
{
	sflow_engine_t *engine;
	uint64_t tics;
	
	engine = rtr_malloc_socket(NULL, sizeof(sflow_engine_t),
			  RTE_CACHE_LINE_SIZE);
	if (engine == NULL)
		return -OUT_OF_MEMORY;

	memset(engine, 0, sizeof(*engine));

	engine->collector_ind = 0;
	engine->worker_state = SFLOW_WRK_STATE_NONE;
	engine->enabled_ports[0].u64 = SFLOW_UNDEF_PORT;
	engine->enabled_ports[1].u64 = SFLOW_UNDEF_PORT;

	/* no collector data, stop all the worker sFlow probes */
	SET_FLAG_DOWN(g_router_params.flags2, ROUTER_PRMS_FL2_SFLOW_WORKERS);
	
	engine->cplane_timer = cplane_timer_get(cplane);
	if (engine->cplane_timer == NULL) {
		rte_free(engine);
		return -CPLANE_NO_TIMERS_LEFT;
	}
	
	/* 
	 * Start a timer to periodically update collector data.
	 */
	tics = SFLOW_CPLANE_PERIODIC_UPDATE * g_router_params.timer_hz;
	rte_timer_reset(engine->cplane_timer, tics, PERIODICAL,
			  CONTROL_PLANE_LCORE_ID, sflow_cplane_update_collector_timer_cb,
			  cplane);	
	
	*out_engine = engine;
	return OK;
}

/*
 * Result:
 *   See the description of save_config_objects_cb_t
 */
int
sflow_save_config(cplane_t *cplane, FILE *config, uint8_t *buf,
		  enum rtr_config_sub_state cfg_state)
{
	uint8_t collector_ind;
	int ret;
	char cmd[CFG_MAX_CMD_LEN];
	const sflow_engine_t *engine;
	const sflow_collector_t *collector;

	(void)buf;

	if (cfg_state == br_cfgs_first_run) {
		ret = fputs(
			RTR_CONF_CMD_TAB"#\n"
			RTR_CONF_CMD_TAB"# sFlow collector\n"
			RTR_CONF_CMD_TAB"#\n",
			config);
		if (ret == EOF)
			return -ERR_CMD_IO_ERR;

		return CFG_RUN_ME_AGAIN;
	}

	engine = cplane->sflow_engine;
	collector_ind = atomic_load_relaxed(&engine->collector_ind);
	collector = &engine->collector[collector_ind];

	/*
	 * Do not emit a collector command until the module has been configured
	 * at least once. The engine allocation is explicit and starts from
	 * zeroed collector slots.
	 */
	if (collector->collector_addr_be != 0 && collector->collector_port_be != 0) {
		ret = snprintf(cmd, CFG_MAX_CMD_LEN,
   			  "sflow collector addr %s port %hu\n",
				  ipv4_to_str(rte_be_to_cpu_32(collector->collector_addr_be)),
				  rte_be_to_cpu_16(collector->collector_port_be));
		assert(ret < CFG_MAX_CMD_LEN);
		ret = config_save_add_cmd(config, cmd);
		if (ret != OK)
			return ret;
	}
	
	for (unsigned i = 0; i < RTE_DIM(engine->enabled_ports); i++) {
		sflow_port_t p;
		
		p.u64 = atomic_load_relaxed(&engine->enabled_ports[i].u64);
		if (p.u64 == SFLOW_UNDEF_PORT)
			continue;
		
		ret = snprintf(cmd, CFG_MAX_CMD_LEN,
   		"sflow probe add port %hu sampling rate rx %hu tx %hu sample %hu\n",
				  p.port,
				  p.sampling_rates[SFLOW_RX],
				  p.sampling_rates[SFLOW_TX], 
				  p.sample_len);
		assert(ret < CFG_MAX_CMD_LEN);
		ret = config_save_add_cmd(config, cmd);
		if (ret != OK)
			return ret;
	}
	
	return OK;
}

int
sflow_worker_init(worker_t *wrk)
{
	sflow_engine_t *engine = g_cplane->sflow_engine;
	uint64_t tics;
		
	memset(&wrk->sflow_worker, 0, sizeof(sflow_worker_t));
	
	/* only one worker core should run the update timer */
	if (rte_lcore_id() != SFLOW_WORKER_COL_UPDATE_CORE)
		return OK;
	
	engine->worker_timer = worker_timer_get(wrk);
	if (engine->worker_timer == NULL)
		return -WORKER_NO_TIMERS_LEFT;
	
	/* 
	 * Start a timer to periodically update collector data.
	 */
	tics = SFLOW_WORKER_PERIODIC_UPDATE * g_router_params.timer_hz;
	rte_timer_reset(engine->worker_timer, tics, PERIODICAL,
			  rte_lcore_id(), sflow_worker_update_collector_timer_cb, wrk);	
	
	return OK;
}

static void
sflow_cli_obj_cb(cli_server_client_t *client,
		  cli_server_msg_line_pool_t *lp, void *state_obj)
{
	const sflow_engine_t *engine;
	const sflow_collector_t *col;

	(void)client;
	(void)state_obj;	
	engine = g_cplane->sflow_engine;
	col = &engine->collector[engine->collector_ind];
	
	CLI_SRV_PRINT1L(lp, "sflow enabled: %s", rtr_bool_to_str(
			  FLAG_UP(g_router_params.flags2,
					ROUTER_PRMS_FL2_SFLOW_GLOBAL_STATE)));
		
	CLI_SRV_PRINT1L(lp, "collector: %s:%hu",
			  ipv4_to_str(rte_be_to_cpu_32(col->collector_addr_be)),
			  rte_be_to_cpu_16(col->collector_port_be));
	
	
	CLI_SRV_PRINT1L(lp, "probe ports:");
	for (unsigned i = 0; i < RTE_DIM(engine->enabled_ports); i++) {
		sflow_port_t p;
		
		p.u64 = atomic_load_relaxed(&engine->enabled_ports[i].u64);
		if (p.u64 == SFLOW_UNDEF_PORT)
			continue;
		
		CLI_SRV_PRINT1L(lp, "  port %hu sampling rate rx %hu tx %hu sample %hu",
				  p.port,
				  p.sampling_rates[SFLOW_RX],
				  p.sampling_rates[SFLOW_TX],
				  p.sample_len);
	}
	
	CLI_SRV_PRINT1L(lp, "");
	CLI_SRV_PRINT1L(lp, "workers:");
		
	for (unsigned i = 0; i < RTE_DIM(g_workers); i++) {
		if (!is_worker_forwarding(i))
			continue;		
		const worker_t *wrk = &g_workers[i];
		
		CLI_SRV_PRINT1L(lp,
				  "  worker %u: rx total %u sent %u, tx total %u sent %u",
				  i,
				  wrk->sflow_worker.counters[SFLOW_RX].total_pkt_cnt,
				  wrk->sflow_worker.counters[SFLOW_RX].seq_num,
				  wrk->sflow_worker.counters[SFLOW_TX].total_pkt_cnt,
				  wrk->sflow_worker.counters[SFLOW_TX].seq_num);
	}

	/* end line */
	cli_server_msg_lp_add_end_line(lp);
}

int
sflow_cli_state_obj_cb(cli_server_client_t *client, void **obj)
{
	CLI_CMD_TYPE1_STATE_OBJ_INIT(client, sflow_cli_obj_cb, obj, 100);
}

static void
sflow_cli_debug_obj_cb(cli_server_client_t *client,
		  cli_server_msg_line_pool_t *lp, void *state_obj)
{
	const sflow_engine_t *engine;
	const sflow_collector_t *col;
	struct rte_ether_hdr *eth;
	uint8_t ind;

	(void)client;
	(void)state_obj;	
	engine = g_cplane->sflow_engine;
	
	ind = atomic_load_acquire(&engine->collector_ind);
	col = &engine->collector[ind];
	
	CLI_SRV_PRINT1L(lp, "sflow index %hhu", ind);
	CLI_SRV_PRINT1L(lp, "egress_port_id %hhu", col->egress_port_id);
	CLI_SRV_PRINT1L(lp, "eth_hdr_len %hhu", col->eth_hdr_len);
	
	eth = (struct rte_ether_hdr *)col->eth_hdr;
	CLI_SRV_PRINT1L(lp, "eth_src %s", mac_addr_to_str(&eth->src_addr));
	CLI_SRV_PRINT1L(lp, "eth_dst %s", mac_addr_to_str(&eth->dst_addr));
	
	CLI_SRV_PRINT1L(lp, "egress_mtu %hu", col->egress_mtu);
	CLI_SRV_PRINT1L(lp, "collector_port %hu",
			  rte_be_to_cpu_16(col->collector_port_be));
	CLI_SRV_PRINT1L(lp, "nexthop %s", ipv4_to_str(col->next_hop_ip));
	CLI_SRV_PRINT1L(lp, "src_addr %s",
			  ipv4_to_str(rte_be_to_cpu_32(col->src_addr_be)));
	CLI_SRV_PRINT1L(lp, "collector_addr %s",
			  ipv4_to_str(rte_be_to_cpu_32(col->collector_addr_be)));	
	
	cli_server_msg_lp_add_end_line(lp);
}

int
sflow_cli_debug_state_obj_cb(cli_server_client_t *client, void **obj)
{
	CLI_CMD_TYPE1_STATE_OBJ_INIT(client, sflow_cli_debug_obj_cb, obj, 30);
}
