/* 
 * File:   sflow_worker.h
 * Author: alex
 *
 * Created on March 30, 2026, 9:23 PM
 */

#ifndef SFLOW_WORKER_H
#define SFLOW_WORKER_H

#include <sys/cdefs.h>

#include "sflow.h"
#include "worker.h"    

__BEGIN_DECLS

#define SFLOW_RX 0
#define SFLOW_TX 1

static inline void
sflow_worker(worker_t *worker, sflow_engine_t *engine,
		  const vif2_t *vif, const struct rte_mbuf *m,
		  const unsigned dir)
{
	unsigned port_ind;
	uint16_t sampling_rate, sflow_port_pkt_cnt;
	sflow_port_t sflow_port;
	sflow_worker_cnt_t *c;
	
	port_ind = 0;
	sflow_port.u64 = atomic_load_relaxed(&engine->enabled_ports[0].u64);
	if (sflow_port.port != vif->port_id) {
		port_ind = 1;
		sflow_port.u64 = atomic_load_relaxed(&engine->enabled_ports[1].u64);
		if (sflow_port.port != vif->port_id)
			return;
	}
	
	c = &worker->sflow_worker.counters[dir];
	c->total_pkt_cnt++;
	sflow_port_pkt_cnt = ++c->pkt_cnt[port_ind];
	
	sampling_rate = sflow_port.sampling_rates[dir];
	if (sflow_port_pkt_cnt < sampling_rate)
		return;
	
	c->pkt_cnt[port_ind] = 0;
	sflow_worker_(worker, engine, vif, m, 
			  sflow_port.sample_len, rte_cpu_to_be_32(sampling_rate), dir);
}

__END_DECLS

#endif /* SFLOW_WORKER_H */

