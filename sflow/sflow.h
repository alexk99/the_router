/* 
 * File:   sflow.h
 * Author: alex
 *
 * Created on March 29, 2026, 3:32 PM
 */

#ifndef SFLOW_H
#define SFLOW_H

#include <sys/cdefs.h>    

__BEGIN_DECLS

#include <stdint.h>
#include <rte_mbuf.h>
#include "eth.h"
#include "config_save.h"

#define SFLOW_DEFAULT_SRC_PORT 6343

#define SFLOW_VER 5
#define SFLOW_ADDR_TYPE_IPV4 1
#define SFLOW_FLOW_SAMPLE 1
#define SFLOW_ETH_PROTO 1

struct sflow_hdr {
	uint32_t ver;
	uint32_t agent_address_type;
	uint32_t agent_address;
	uint32_t sub_agent_id;
	uint32_t seq_num;
	uint32_t sys_uptime;
	uint32_t num_samples;
	uint8_t samples[0];
}
__attribute__((__packed__));

struct sflow_sample_record {
	uint32_t hdr;
	uint32_t flow_data_len;
	uint32_t header_protocol;
	uint32_t frame_len; /* Original length of packet before sampling */
	uint32_t payload_stripped;
	uint32_t sample_header_len;
	uint8_t sample_header[0];
}
__attribute__((__packed__));

#define FLOW_HDR_LEN 8 /* hdr + len fields */

struct sflow_sample {
	uint32_t hdr;
	uint32_t len;
	uint32_t seq_num;
	uint32_t source_id;
	uint32_t sampling_rate;
	uint32_t sample_pool;
	uint32_t dropped_pkts;
	uint32_t intput_iface;
	uint32_t output_iface;
	uint32_t flow_record;
	struct sflow_sample_record sample_rec;
}
__attribute__((__packed__));
		 
typedef struct sflow_collector
{
	uint8_t egress_port_id;
	uint8_t eth_hdr_len;
	uint8_t eth_hdr[sizeof(struct eth_qinq_hdr)] __rte_aligned(2);
	uint16_t egress_mtu;
	uint16_t collector_port_be;
	uint32_t next_hop_ip;
	uint32_t src_addr_be;
	uint32_t collector_addr_be;
}
sflow_collector_t;

#define SFLOW_MAX_PROBE_PORTS 2

typedef struct sflow_worker_cnt
{
	uint32_t total_pkt_cnt;
	uint32_t seq_num;
	uint16_t pkt_cnt[SFLOW_MAX_PROBE_PORTS]; /* per sflow port packet counter */
}
sflow_worker_cnt_t;

typedef struct sflow_worker
{
	/* rx, tx counters */
	sflow_worker_cnt_t counters[2];
}
sflow_worker_t;

/* worker core to run the timer updating collector data */
#define SFLOW_WORKER_COL_UPDATE_CORE 1

#define SFLOW_CPLANE_PERIODIC_UPDATE 5 /* sec */
#define SFLOW_WORKER_PERIODIC_UPDATE 5 /* sec */

/* no job for worker */
#define SFLOW_WRK_STATE_NONE      0
/* worker is either running the update timer or going to run it */
#define SFLOW_WRK_STATE_RUN       1
/* worker has completed the update timer job */
#define SFLOW_WRK_STATE_COMPLETE  2

#define SFLOW_UNDEF_PORT UINT64_MAX

typedef union sflow_port
{
	struct {
		uint16_t port;
		uint16_t sample_len;
		/* rx, tx sampling rate */
		uint16_t sampling_rates[2];
	};
	uint64_t u64;
}
sflow_port_t;

typedef struct sflow_engine
{
	uint8_t worker_state;
	
	/* index of eth_hdrs read to use data */
	uint8_t collector_ind;
	
	/* ports to collect traffic from */
	sflow_port_t enabled_ports[SFLOW_MAX_PROBE_PORTS];
	
	/* 
	 * Collector.
	 * 2 copies for multi-thread safety
	 *   the slot at collector_ind is the active slot used by workers;
	 *   the slot at !collector_ind is the shadow slot being updated
	 *			by cplane and worker timer callbacks 
	 *				sflow_worker_update_collector_timer_cb
	 *			   sflow_cplane_update_collector_timer_cb
	 * 
	 * Concurrency:
	 *   once collector[collector_ind] is ready,
	 *   flag ROUTER_PRMS_FL2_SFLOW is UP in g_router_params.flags.
	 * 
	 * collector_ind can be changed only after collector[!collector_ind]
	 * has been fully initialized.
 	 */
	sflow_collector_t collector[2];
	
	struct rte_timer *worker_timer;
	struct rte_timer *cplane_timer;
}
sflow_engine_t;

typedef struct cplane cplane_t;
typedef struct vif2 vif2_t;
typedef struct worker worker_t;
typedef struct cli_server_client cli_server_client_t;
struct rconf_port_list;

void
sflow_worker_(worker_t *worker, sflow_engine_t *engine,
		  const vif2_t *ingress_vif, const struct rte_mbuf *m,
		  uint16_t sample_len, uint32_t sampling_rate_be,
		  const unsigned dir);

int
sflow_collector_set(cplane_t *cplane, uint32_t addr, uint16_t port);

int
sflow_probe_port_add(cplane_t *cplane, uint8_t port, 
		  uint16_t rx_sampling_rate, uint16_t tx_sampling_rate,
		  uint16_t sample_len);

int
sflow_probe_port_del(cplane_t *cplane, uint8_t port);


int
sflow_engine_init(cplane_t *cplane, sflow_engine_t **out_engine);

int
sflow_worker_init(worker_t *wrk);

int
sflow_cli_state_obj_cb(cli_server_client_t *client, void **obj);

int
sflow_cli_debug_state_obj_cb(cli_server_client_t *client, void **obj);

int
sflow_save_config(cplane_t *cplane, FILE *config, uint8_t *buf,
		  enum rtr_config_sub_state cfg_state);

__END_DECLS

#endif /* SFLOW_H */
