#ifndef IPIFSTATSTABLE_DATA_GET_H
#define IPIFSTATSTABLE_DATA_GET_H
int ipIfStatsInReceives_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsInReceives_val_ptr);

int ipIfStatsHCInReceives_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCInReceives_val_ptr);

int ipIfStatsInOctets_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                          u_long *ipIfStatsInOctets_val_ptr);

int ipIfStatsHCInOctets_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            unsigned long long *ipIfStatsHCInOctets_val_ptr);

int ipIfStatsInHdrErrors_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             u_long *ipIfStatsInHdrErrors_val_ptr);

int ipIfStatsInNoRoutes_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsInNoRoutes_val_ptr);

int ipIfStatsInAddrErrors_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipIfStatsInAddrErrors_val_ptr);

int ipIfStatsInUnknownProtos_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipIfStatsInUnknownProtos_val_ptr);

int ipIfStatsInTruncatedPkts_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipIfStatsInTruncatedPkts_val_ptr);

int ipIfStatsInForwDatagrams_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipIfStatsInForwDatagrams_val_ptr);

int ipIfStatsHCInForwDatagrams_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCInForwDatagrams_val_ptr);

int ipIfStatsReasmReqds_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsReasmReqds_val_ptr);

int ipIfStatsReasmOKs_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                          u_long *ipIfStatsReasmOKs_val_ptr);

int ipIfStatsReasmFails_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsReasmFails_val_ptr);

int ipIfStatsInDiscards_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsInDiscards_val_ptr);

int ipIfStatsInDelivers_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsInDelivers_val_ptr);

int ipIfStatsHCInDelivers_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCInDelivers_val_ptr);

int ipIfStatsOutRequests_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             u_long *ipIfStatsOutRequests_val_ptr);

int ipIfStatsHCOutRequests_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCOutRequests_val_ptr);

int ipIfStatsOutForwDatagrams_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipIfStatsOutForwDatagrams_val_ptr);

int ipIfStatsHCOutForwDatagrams_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCOutForwDatagrams_val_ptr);

int ipIfStatsOutDiscards_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             u_long *ipIfStatsOutDiscards_val_ptr);

int ipIfStatsOutFragReqds_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipIfStatsOutFragReqds_val_ptr);

int ipIfStatsOutFragOKs_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsOutFragOKs_val_ptr);

int ipIfStatsOutFragFails_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipIfStatsOutFragFails_val_ptr);

int ipIfStatsOutFragCreates_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipIfStatsOutFragCreates_val_ptr);

int ipIfStatsOutTransmits_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipIfStatsOutTransmits_val_ptr);

int ipIfStatsHCOutTransmits_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCOutTransmits_val_ptr);

int ipIfStatsOutOctets_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                           u_long *ipIfStatsOutOctets_val_ptr);

int ipIfStatsHCOutOctets_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             unsigned long long *ipIfStatsHCOutOctets_val_ptr);

int ipIfStatsInMcastPkts_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             u_long *ipIfStatsInMcastPkts_val_ptr);

int ipIfStatsHCInMcastPkts_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCInMcastPkts_val_ptr);

int ipIfStatsInMcastOctets_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                               u_long *ipIfStatsInMcastOctets_val_ptr);

int ipIfStatsHCInMcastOctets_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCInMcastOctets_val_ptr);

int ipIfStatsOutMcastPkts_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipIfStatsOutMcastPkts_val_ptr);

int ipIfStatsHCOutMcastPkts_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCOutMcastPkts_val_ptr);

int ipIfStatsOutMcastOctets_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipIfStatsOutMcastOctets_val_ptr);

int ipIfStatsHCOutMcastOctets_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCOutMcastOctets_val_ptr);

int ipIfStatsInBcastPkts_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             u_long *ipIfStatsInBcastPkts_val_ptr);

int ipIfStatsHCInBcastPkts_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCInBcastPkts_val_ptr);

int ipIfStatsOutBcastPkts_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipIfStatsOutBcastPkts_val_ptr);

int ipIfStatsHCOutBcastPkts_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCOutBcastPkts_val_ptr);

int ipIfStatsDiscontinuityTime_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                   long *ipIfStatsDiscontinuityTime_val_ptr);

int ipIfStatsRefreshRate_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             u_long *ipIfStatsRefreshRate_val_ptr);

int ipIfStatsTable_indexes_set_tbl_idx(ipIfStatsTable_mib_index *tbl_idx,
                                       long ipIfStatsIPVersion_val,
                                       long ipIfStatsIfIndex_val);

int ipIfStatsTable_indexes_set(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                               long ipIfStatsIPVersion_val,
                               long ipIfStatsIfIndex_val);
#endif
