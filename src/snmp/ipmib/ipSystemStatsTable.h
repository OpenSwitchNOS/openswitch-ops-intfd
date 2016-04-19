#ifndef IPSYSTEMSTATSTABLE_H
#define IPSYSTEMSTATSTABLE_H
#include <net-snmp/library/asn1.h>
#include "ipSystemStatsTable_oids.h"
#include "ipSystemStatsTable_enums.h"

void init_ipSystemStatsTable(void);
void shutdown_ipSystemStatsTable(void);

typedef netsnmp_data_list ipSystemStatsTable_registration;

typedef struct ipSystemStatsTable_data_s {
    u_long ipSystemStatsInReceives;
    unsigned long long ipSystemStatsHCInReceives;
    u_long ipSystemStatsInOctets;
    unsigned long long ipSystemStatsHCInOctets;
    u_long ipSystemStatsInHdrErrors;
    u_long ipSystemStatsInNoRoutes;
    u_long ipSystemStatsInAddrErrors;
    u_long ipSystemStatsInUnknownProtos;
    u_long ipSystemStatsInTruncatedPkts;
    u_long ipSystemStatsInForwDatagrams;
    unsigned long long ipSystemStatsHCInForwDatagrams;
    u_long ipSystemStatsReasmReqds;
    u_long ipSystemStatsReasmOKs;
    u_long ipSystemStatsReasmFails;
    u_long ipSystemStatsInDiscards;
    u_long ipSystemStatsInDelivers;
    unsigned long long ipSystemStatsHCInDelivers;
    u_long ipSystemStatsOutRequests;
    unsigned long long ipSystemStatsHCOutRequests;
    u_long ipSystemStatsOutNoRoutes;
    u_long ipSystemStatsOutForwDatagrams;
    unsigned long long ipSystemStatsHCOutForwDatagrams;
    u_long ipSystemStatsOutDiscards;
    u_long ipSystemStatsOutFragReqds;
    u_long ipSystemStatsOutFragOKs;
    u_long ipSystemStatsOutFragFails;
    u_long ipSystemStatsOutFragCreates;
    u_long ipSystemStatsOutTransmits;
    unsigned long long ipSystemStatsHCOutTransmits;
    u_long ipSystemStatsOutOctets;
    unsigned long long ipSystemStatsHCOutOctets;
    u_long ipSystemStatsInMcastPkts;
    unsigned long long ipSystemStatsHCInMcastPkts;
    u_long ipSystemStatsInMcastOctets;
    unsigned long long ipSystemStatsHCInMcastOctets;
    u_long ipSystemStatsOutMcastPkts;
    unsigned long long ipSystemStatsHCOutMcastPkts;
    u_long ipSystemStatsOutMcastOctets;
    unsigned long long ipSystemStatsHCOutMcastOctets;
    u_long ipSystemStatsInBcastPkts;
    unsigned long long ipSystemStatsHCInBcastPkts;
    u_long ipSystemStatsOutBcastPkts;
    unsigned long long ipSystemStatsHCOutBcastPkts;
    long ipSystemStatsDiscontinuityTime;
    u_long ipSystemStatsRefreshRate;
} ipSystemStatsTable_data;

typedef struct ipSystemStatsTable_mib_index_s {
    long ipSystemStatsIPVersion;
} ipSystemStatsTable_mib_index;

typedef struct ipSystemStatsTable_rowreq_ctx_s {
    netsnmp_index oid_idx;
    oid oid_tmp[MAX_OID_LEN];
    ipSystemStatsTable_mib_index tbl_idx;
    ipSystemStatsTable_data data;
    u_int rowreq_flags;
    netsnmp_data_list *ipSystemStatsTable_data_list;
} ipSystemStatsTable_rowreq_ctx;

typedef struct ipSystemStatsTable_ref_rowreq_ctx_s {
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx;
} ipSystemStatsTable_ref_rowreq_ctx;

int ipSystemStatsTable_pre_request(
    ipSystemStatsTable_registration *user_context);
int ipSystemStatsTable_post_request(
    ipSystemStatsTable_registration *user_context, int rc);
int ipSystemStatsTable_rowreq_ctx_init(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx, void *user_init_ctx);
void ipSystemStatsTable_rowreq_ctx_cleanup(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx);
ipSystemStatsTable_rowreq_ctx *
ipSystemStatsTable_row_find_by_mib_index(ipSystemStatsTable_mib_index *mib_idx);
extern const oid ipSystemStatsTable_oid[];
extern const int ipSystemStatsTable_oid_size;
#include "ipSystemStatsTable_interface.h"
#include "ipSystemStatsTable_data_access.h"
#include "ipSystemStatsTable_data_get.h"
#include "ipSystemStatsTable_data_set.h"

#endif
