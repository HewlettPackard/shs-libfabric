/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * (c) Copyright 2021-2023 Hewlett Packard Enterprise Development LP
 */

union nicaddr {
	uint64_t value;
	struct {
		uint64_t nic:20;	// 20-bit CXI NIC address
		uint64_t net:28;	// 28-bit network route
		uint64_t hsn:2;		// up to 4 CXI chips per node
		uint64_t rank:14;	// up to 16k ranks
	} __attribute__((__packed__));
};
#define	NICSIZE	(sizeof(union nicaddr))

/* These are initialized by frmwk_init() */
int frmwk_nics_per_rank;
int frmwk_numranks;
int frmwk_numnics;
int frmwk_rank;

/* This is initialized by frmwk_populate_av() */
union nicaddr *frmwk_nics;

char *cxit_node;
char *cxit_service;
uint64_t cxit_flags;
struct fi_info *cxit_fi_hints;
struct fi_info *cxit_fi;

struct fid_fabric *cxit_fabric;
struct fid_domain *cxit_domain;
struct fi_cxi_dom_ops *dom_ops;

struct fid_ep *cxit_ep;
struct fi_eq_attr cxit_eq_attr;
uint64_t cxit_eq_bind_flags;
struct fid_eq *cxit_eq;

struct fi_cq_attr cxit_rx_cq_attr;
uint64_t cxit_rx_cq_bind_flags;
struct fid_cq *cxit_rx_cq;

struct fi_cq_attr cxit_tx_cq_attr;
uint64_t cxit_tx_cq_bind_flags;
struct fid_cq *cxit_tx_cq;

fi_addr_t cxit_ep_fi_addr;

struct fi_cntr_attr cxit_cntr_attr;
struct fid_cntr *cxit_send_cntr;
struct fid_cntr *cxit_recv_cntr;
struct fid_cntr *cxit_read_cntr;
struct fid_cntr *cxit_write_cntr;
struct fid_cntr *cxit_rem_cntr;

struct fi_av_attr cxit_av_attr;
struct fid_av *cxit_av;

int cxit_n_ifs;
struct fid_av_set *cxit_av_set;
struct fid_mc *cxit_mc;
fi_addr_t cxit_mc_addr;

int frmwk_allgather(size_t size, void *data, void *rslt);
int frmwk_barrier(void);
int frmwk_gather_nics(void);
int frmwk_nic_addr(int rank, int hsn);

void frmwk_init(bool quiet);
void frmwk_term(void);
int frmwk_init_libfabric(void);
void frmwk_free_libfabric(void);
int frmwk_check_env(int minranks);
int frmwk_populate_av(fi_addr_t **fiaddr, size_t *size);
int frmwk_errmsg(int ret, const char *fmt, ...)
	__attribute__((format(__printf__, 2, 3)));
int frmwk_log0(const char *fmt, ...)
	__attribute__((format(__printf__, 1, 2)));
int frmwk_log(const char *fmt, ...)
	__attribute__((format(__printf__, 1, 2)));