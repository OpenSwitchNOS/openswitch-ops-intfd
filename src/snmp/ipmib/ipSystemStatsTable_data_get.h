#ifndef IPSYSTEMSTATSTABLE_DATA_GET_H
#define IPSYSTEMSTATSTABLE_DATA_GET_H
int ipSystemStatsInReceives_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsInReceives_val_ptr);

int ipSystemStatsHCInReceives_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInReceives_val_ptr);

int ipSystemStatsInOctets_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipSystemStatsInOctets_val_ptr);

int ipSystemStatsHCInOctets_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInOctets_val_ptr);

int ipSystemStatsInHdrErrors_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsInHdrErrors_val_ptr);

int ipSystemStatsInNoRoutes_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsInNoRoutes_val_ptr);

int ipSystemStatsInAddrErrors_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipSystemStatsInAddrErrors_val_ptr);

int ipSystemStatsInUnknownProtos_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    u_long *ipSystemStatsInUnknownProtos_val_ptr);

int ipSystemStatsInTruncatedPkts_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    u_long *ipSystemStatsInTruncatedPkts_val_ptr);

int ipSystemStatsInForwDatagrams_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    u_long *ipSystemStatsInForwDatagrams_val_ptr);

int ipSystemStatsHCInForwDatagrams_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInForwDatagrams_val_ptr);

int ipSystemStatsReasmReqds_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsReasmReqds_val_ptr);

int ipSystemStatsReasmOKs_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipSystemStatsReasmOKs_val_ptr);

int ipSystemStatsReasmFails_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsReasmFails_val_ptr);

int ipSystemStatsInDiscards_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsInDiscards_val_ptr);

int ipSystemStatsInDelivers_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsInDelivers_val_ptr);

int ipSystemStatsHCInDelivers_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInDelivers_val_ptr);

int ipSystemStatsOutRequests_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsOutRequests_val_ptr);

int ipSystemStatsHCOutRequests_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutRequests_val_ptr);

int ipSystemStatsOutNoRoutes_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsOutNoRoutes_val_ptr);

int ipSystemStatsOutForwDatagrams_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    u_long *ipSystemStatsOutForwDatagrams_val_ptr);

int ipSystemStatsHCOutForwDatagrams_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutForwDatagrams_val_ptr);

int ipSystemStatsOutDiscards_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsOutDiscards_val_ptr);

int ipSystemStatsOutFragReqds_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipSystemStatsOutFragReqds_val_ptr);

int ipSystemStatsOutFragOKs_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsOutFragOKs_val_ptr);

int ipSystemStatsOutFragFails_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipSystemStatsOutFragFails_val_ptr);

int ipSystemStatsOutFragCreates_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    u_long *ipSystemStatsOutFragCreates_val_ptr);

int ipSystemStatsOutTransmits_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipSystemStatsOutTransmits_val_ptr);

int ipSystemStatsHCOutTransmits_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutTransmits_val_ptr);

int ipSystemStatsOutOctets_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                               u_long *ipSystemStatsOutOctets_val_ptr);

int ipSystemStatsHCOutOctets_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutOctets_val_ptr);

int ipSystemStatsInMcastPkts_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsInMcastPkts_val_ptr);

int ipSystemStatsHCInMcastPkts_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInMcastPkts_val_ptr);

int ipSystemStatsInMcastOctets_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                   u_long *ipSystemStatsInMcastOctets_val_ptr);

int ipSystemStatsHCInMcastOctets_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInMcastOctets_val_ptr);

int ipSystemStatsOutMcastPkts_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipSystemStatsOutMcastPkts_val_ptr);

int ipSystemStatsHCOutMcastPkts_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutMcastPkts_val_ptr);

int ipSystemStatsOutMcastOctets_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    u_long *ipSystemStatsOutMcastOctets_val_ptr);

int ipSystemStatsHCOutMcastOctets_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutMcastOctets_val_ptr);

int ipSystemStatsInBcastPkts_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsInBcastPkts_val_ptr);

int ipSystemStatsHCInBcastPkts_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInBcastPkts_val_ptr);

int ipSystemStatsOutBcastPkts_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipSystemStatsOutBcastPkts_val_ptr);

int ipSystemStatsHCOutBcastPkts_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutBcastPkts_val_ptr);

int ipSystemStatsDiscontinuityTime_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    long *ipSystemStatsDiscontinuityTime_val_ptr);

int ipSystemStatsRefreshRate_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsRefreshRate_val_ptr);

int ipSystemStatsTable_indexes_set_tbl_idx(
    ipSystemStatsTable_mib_index *tbl_idx, long ipSystemStatsIPVersion_val);

int ipSystemStatsTable_indexes_set(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                   long ipSystemStatsIPVersion_val);
#endif
