#ifndef IFXTABLE_DATA_GET_H
#define IFXTABLE_DATA_GET_H
int ifName_get(ifXTable_rowreq_ctx *rowreq_ctx, char **ifName_val_ptr_ptr,
               size_t *ifName_val_ptr_len_ptr);

int ifInMulticastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                          u_long *ifInMulticastPkts_val_ptr);

int ifInBroadcastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                          u_long *ifInBroadcastPkts_val_ptr);

int ifOutMulticastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                           u_long *ifOutMulticastPkts_val_ptr);

int ifOutBroadcastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                           u_long *ifOutBroadcastPkts_val_ptr);

int ifHCInOctets_get(ifXTable_rowreq_ctx *rowreq_ctx,
                     U64 *ifHCInOctets_val_ptr);

int ifHCInUcastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                        U64 *ifHCInUcastPkts_val_ptr);

int ifHCInMulticastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                            U64 *ifHCInMulticastPkts_val_ptr);

int ifHCInBroadcastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                            U64 *ifHCInBroadcastPkts_val_ptr);

int ifHCOutOctets_get(ifXTable_rowreq_ctx *rowreq_ctx,
                      U64 *ifHCOutOctets_val_ptr);

int ifHCOutUcastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                         U64 *ifHCOutUcastPkts_val_ptr);

int ifHCOutMulticastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                             U64 *ifHCOutMulticastPkts_val_ptr);

int ifHCOutBroadcastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                             U64 *ifHCOutBroadcastPkts_val_ptr);

int ifLinkUpDownTrapEnable_get(ifXTable_rowreq_ctx *rowreq_ctx,
                               long *ifLinkUpDownTrapEnable_val_ptr);

int ifHighSpeed_get(ifXTable_rowreq_ctx *rowreq_ctx,
                    u_long *ifHighSpeed_val_ptr);

int ifPromiscuousMode_get(ifXTable_rowreq_ctx *rowreq_ctx,
                          long *ifPromiscuousMode_val_ptr);

int ifConnectorPresent_get(ifXTable_rowreq_ctx *rowreq_ctx,
                           long *ifConnectorPresent_val_ptr);

int ifAlias_get(ifXTable_rowreq_ctx *rowreq_ctx, char **ifAlias_val_ptr_ptr,
                size_t *ifAlias_val_ptr_len_ptr);

int ifCounterDiscontinuityTime_get(ifXTable_rowreq_ctx *rowreq_ctx,
                                   long *ifCounterDiscontinuityTime_val_ptr);

int ifXTable_indexes_set_tbl_idx(ifXTable_mib_index *tbl_idx, long ifIndex_val);

int ifXTable_indexes_set(ifXTable_rowreq_ctx *rowreq_ctx, long ifIndex_val);
#endif
