#ifndef IFTABLE_DATA_GET_H
#define IFTABLE_DATA_GET_H
int ifDescr_get(ifTable_rowreq_ctx *rowreq_ctx, char **ifDescr_val_ptr_ptr,
                size_t *ifDescr_val_ptr_len_ptr);

int ifType_get(ifTable_rowreq_ctx *rowreq_ctx, long *ifType_val_ptr);

int ifMtu_get(ifTable_rowreq_ctx *rowreq_ctx, long *ifMtu_val_ptr);

int ifSpeed_get(ifTable_rowreq_ctx *rowreq_ctx, u_long *ifSpeed_val_ptr);

int ifPhysAddress_get(ifTable_rowreq_ctx *rowreq_ctx,
                      char **ifPhysAddress_val_ptr_ptr,
                      size_t *ifPhysAddress_val_ptr_len_ptr);

int ifAdminStatus_get(ifTable_rowreq_ctx *rowreq_ctx,
                      long *ifAdminStatus_val_ptr);

int ifOperStatus_get(ifTable_rowreq_ctx *rowreq_ctx,
                     long *ifOperStatus_val_ptr);

int ifLastChange_get(ifTable_rowreq_ctx *rowreq_ctx,
                     long *ifLastChange_val_ptr);

int ifInOctets_get(ifTable_rowreq_ctx *rowreq_ctx, u_long *ifInOctets_val_ptr);

int ifInUcastPkts_get(ifTable_rowreq_ctx *rowreq_ctx,
                      u_long *ifInUcastPkts_val_ptr);

int ifInNUcastPkts_get(ifTable_rowreq_ctx *rowreq_ctx,
                       u_long *ifInNUcastPkts_val_ptr);

int ifInDiscards_get(ifTable_rowreq_ctx *rowreq_ctx,
                     u_long *ifInDiscards_val_ptr);

int ifInErrors_get(ifTable_rowreq_ctx *rowreq_ctx, u_long *ifInErrors_val_ptr);

int ifInUnknownProtos_get(ifTable_rowreq_ctx *rowreq_ctx,
                          u_long *ifInUnknownProtos_val_ptr);

int ifOutOctets_get(ifTable_rowreq_ctx *rowreq_ctx,
                    u_long *ifOutOctets_val_ptr);

int ifOutUcastPkts_get(ifTable_rowreq_ctx *rowreq_ctx,
                       u_long *ifOutUcastPkts_val_ptr);

int ifOutNUcastPkts_get(ifTable_rowreq_ctx *rowreq_ctx,
                        u_long *ifOutNUcastPkts_val_ptr);

int ifOutDiscards_get(ifTable_rowreq_ctx *rowreq_ctx,
                      u_long *ifOutDiscards_val_ptr);

int ifOutErrors_get(ifTable_rowreq_ctx *rowreq_ctx,
                    u_long *ifOutErrors_val_ptr);

int ifOutQLen_get(ifTable_rowreq_ctx *rowreq_ctx, u_long *ifOutQLen_val_ptr);

int ifSpecific_get(ifTable_rowreq_ctx *rowreq_ctx, oid **ifSpecific_val_ptr_ptr,
                   size_t *ifSpecific_val_ptr_len_ptr);

int ifTable_indexes_set_tbl_idx(ifTable_mib_index *tbl_idx, long ifIndex_val);

int ifTable_indexes_set(ifTable_rowreq_ctx *rowreq_ctx, long ifIndex_val);
#endif