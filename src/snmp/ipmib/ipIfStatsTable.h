#ifndef IPIFSTATSTABLE_H
#define IPIFSTATSTABLE_H
#include <net-snmp/library/asn1.h>
#include "ipIfStatsTable_oids.h"
#include "ipIfStatsTable_enums.h"

void init_ipIfStatsTable(void);
void shutdown_ipIfStatsTable(void);

typedef netsnmp_data_list ipIfStatsTable_registration;

typedef struct ipIfStatsTable_data_s {
    u_long ipIfStatsInReceives;
    unsigned long long ipIfStatsHCInReceives;
    u_long ipIfStatsInOctets;
    unsigned long long ipIfStatsHCInOctets;
    u_long ipIfStatsInHdrErrors;
    u_long ipIfStatsInNoRoutes;
    u_long ipIfStatsInAddrErrors;
    u_long ipIfStatsInUnknownProtos;
    u_long ipIfStatsInTruncatedPkts;
    u_long ipIfStatsInForwDatagrams;
    unsigned long long ipIfStatsHCInForwDatagrams;
    u_long ipIfStatsReasmReqds;
    u_long ipIfStatsReasmOKs;
    u_long ipIfStatsReasmFails;
    u_long ipIfStatsInDiscards;
    u_long ipIfStatsInDelivers;
    unsigned long long ipIfStatsHCInDelivers;
    u_long ipIfStatsOutRequests;
    unsigned long long ipIfStatsHCOutRequests;
    u_long ipIfStatsOutForwDatagrams;
    unsigned long long ipIfStatsHCOutForwDatagrams;
    u_long ipIfStatsOutDiscards;
    u_long ipIfStatsOutFragReqds;
    u_long ipIfStatsOutFragOKs;
    u_long ipIfStatsOutFragFails;
    u_long ipIfStatsOutFragCreates;
    u_long ipIfStatsOutTransmits;
    unsigned long long ipIfStatsHCOutTransmits;
    u_long ipIfStatsOutOctets;
    unsigned long long ipIfStatsHCOutOctets;
    u_long ipIfStatsInMcastPkts;
    unsigned long long ipIfStatsHCInMcastPkts;
    u_long ipIfStatsInMcastOctets;
    unsigned long long ipIfStatsHCInMcastOctets;
    u_long ipIfStatsOutMcastPkts;
    unsigned long long ipIfStatsHCOutMcastPkts;
    u_long ipIfStatsOutMcastOctets;
    unsigned long long ipIfStatsHCOutMcastOctets;
    u_long ipIfStatsInBcastPkts;
    unsigned long long ipIfStatsHCInBcastPkts;
    u_long ipIfStatsOutBcastPkts;
    unsigned long long ipIfStatsHCOutBcastPkts;
    long ipIfStatsDiscontinuityTime;
    u_long ipIfStatsRefreshRate;
} ipIfStatsTable_data;

typedef struct ipIfStatsTable_mib_index_s {
    long ipIfStatsIPVersion;
    long ipIfStatsIfIndex;
} ipIfStatsTable_mib_index;

typedef struct ipIfStatsTable_rowreq_ctx_s {
    netsnmp_index oid_idx;
    oid oid_tmp[MAX_OID_LEN];
    ipIfStatsTable_mib_index tbl_idx;
    ipIfStatsTable_data data;
    u_int rowreq_flags;
    netsnmp_data_list *ipIfStatsTable_data_list;
} ipIfStatsTable_rowreq_ctx;

typedef struct ipIfStatsTable_ref_rowreq_ctx_s {
    ipIfStatsTable_rowreq_ctx *rowreq_ctx;
} ipIfStatsTable_ref_rowreq_ctx;

int ipIfStatsTable_pre_request(ipIfStatsTable_registration *user_context);
int ipIfStatsTable_post_request(ipIfStatsTable_registration *user_context,
                                int rc);
int ipIfStatsTable_rowreq_ctx_init(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                   void *user_init_ctx);
void ipIfStatsTable_rowreq_ctx_cleanup(ipIfStatsTable_rowreq_ctx *rowreq_ctx);
ipIfStatsTable_rowreq_ctx *
ipIfStatsTable_row_find_by_mib_index(ipIfStatsTable_mib_index *mib_idx);
extern const oid ipIfStatsTable_oid[];
extern const int ipIfStatsTable_oid_size;
#include "ipIfStatsTable_interface.h"
#include "ipIfStatsTable_data_access.h"
#include "ipIfStatsTable_data_get.h"
#include "ipIfStatsTable_data_set.h"

#endif
