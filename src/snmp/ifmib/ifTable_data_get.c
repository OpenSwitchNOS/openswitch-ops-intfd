#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "ifTable.h"

int ifTable_indexes_set_tbl_idx(ifTable_mib_index *tbl_idx, long ifIndex_val) {
    DEBUGMSGTL(("verbose:ifTable:ifTable_indexes_set_tbl_idx", "called\n"));

    tbl_idx->ifIndex = ifIndex_val;
    return MFD_SUCCESS;
}

int ifTable_indexes_set(ifTable_rowreq_ctx *rowreq_ctx, long ifIndex_val) {
    DEBUGMSGTL(("verbose:ifTable:ifTable_indexes_set", "called\n"));
    if (MFD_SUCCESS !=
        ifTable_indexes_set_tbl_idx(&rowreq_ctx->tbl_idx, ifIndex_val)) {
        return MFD_ERROR;
    }
    rowreq_ctx->oid_idx.len = sizeof(rowreq_ctx->oid_tmp) / sizeof(oid);
    if (0 != ifTable_index_to_oid(&rowreq_ctx->oid_idx, &rowreq_ctx->tbl_idx)) {
        return MFD_ERROR;
    }
    return MFD_SUCCESS;
}

int ifDescr_get(ifTable_rowreq_ctx *rowreq_ctx, char **ifDescr_val_ptr_ptr,
                size_t *ifDescr_val_ptr_len_ptr) {
    netsnmp_assert((NULL != ifDescr_val_ptr_ptr) &&
                   (NULL != *ifDescr_val_ptr_ptr));
    netsnmp_assert(NULL != ifDescr_val_ptr_len_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifDescr_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    if ((NULL == (*ifDescr_val_ptr_ptr)) ||
        ((*ifDescr_val_ptr_len_ptr) < (rowreq_ctx->data.ifDescr_len *
                                       sizeof(rowreq_ctx->data.ifDescr[0])))) {
        (*ifDescr_val_ptr_ptr) = malloc(rowreq_ctx->data.ifDescr_len *
                                        sizeof(rowreq_ctx->data.ifDescr[0]));
        if (NULL == (*ifDescr_val_ptr_ptr)) {
            snmp_log(LOG_ERR,
                     "could not allocate memory (rowreq_ctx->data.ifDescr)\n");
            return MFD_ERROR;
        }
    }
    (*ifDescr_val_ptr_len_ptr) =
        rowreq_ctx->data.ifDescr_len * sizeof(rowreq_ctx->data.ifDescr[0]);
    memcpy((*ifDescr_val_ptr_ptr), rowreq_ctx->data.ifDescr,
           rowreq_ctx->data.ifDescr_len * sizeof(rowreq_ctx->data.ifDescr[0]));
    return MFD_SUCCESS;
}

int ifType_get(ifTable_rowreq_ctx *rowreq_ctx, long *ifType_val_ptr) {
    netsnmp_assert(NULL != ifType_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifType_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifType_val_ptr) = rowreq_ctx->data.ifType;
    return MFD_SUCCESS;
}

int ifMtu_get(ifTable_rowreq_ctx *rowreq_ctx, long *ifMtu_val_ptr) {
    netsnmp_assert(NULL != ifMtu_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifMtu_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifMtu_val_ptr) = rowreq_ctx->data.ifMtu;
    return MFD_SUCCESS;
}

int ifSpeed_get(ifTable_rowreq_ctx *rowreq_ctx, u_long *ifSpeed_val_ptr) {
    netsnmp_assert(NULL != ifSpeed_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifSpeed_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifSpeed_val_ptr) = rowreq_ctx->data.ifSpeed;
    return MFD_SUCCESS;
}

int ifPhysAddress_get(ifTable_rowreq_ctx *rowreq_ctx,
                      char **ifPhysAddress_val_ptr_ptr,
                      size_t *ifPhysAddress_val_ptr_len_ptr) {
    netsnmp_assert((NULL != ifPhysAddress_val_ptr_ptr) &&
                   (NULL != *ifPhysAddress_val_ptr_ptr));
    netsnmp_assert(NULL != ifPhysAddress_val_ptr_len_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifPhysAddress_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    if ((NULL == (*ifPhysAddress_val_ptr_ptr)) ||
        ((*ifPhysAddress_val_ptr_len_ptr) <
         (rowreq_ctx->data.ifPhysAddress_len *
          sizeof(rowreq_ctx->data.ifPhysAddress[0])))) {
        (*ifPhysAddress_val_ptr_ptr) =
            malloc(rowreq_ctx->data.ifPhysAddress_len *
                   sizeof(rowreq_ctx->data.ifPhysAddress[0]));
        if (NULL == (*ifPhysAddress_val_ptr_ptr)) {
            snmp_log(
                LOG_ERR,
                "could not allocate memory (rowreq_ctx->data.ifPhysAddress)\n");
            return MFD_ERROR;
        }
    }
    (*ifPhysAddress_val_ptr_len_ptr) =
        rowreq_ctx->data.ifPhysAddress_len *
        sizeof(rowreq_ctx->data.ifPhysAddress[0]);
    memcpy((*ifPhysAddress_val_ptr_ptr), rowreq_ctx->data.ifPhysAddress,
           rowreq_ctx->data.ifPhysAddress_len *
               sizeof(rowreq_ctx->data.ifPhysAddress[0]));
    return MFD_SUCCESS;
}

int ifAdminStatus_get(ifTable_rowreq_ctx *rowreq_ctx,
                      long *ifAdminStatus_val_ptr) {
    netsnmp_assert(NULL != ifAdminStatus_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifAdminStatus_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifAdminStatus_val_ptr) = rowreq_ctx->data.ifAdminStatus;
    return MFD_SUCCESS;
}

int ifOperStatus_get(ifTable_rowreq_ctx *rowreq_ctx,
                     long *ifOperStatus_val_ptr) {
    netsnmp_assert(NULL != ifOperStatus_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifOperStatus_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifOperStatus_val_ptr) = rowreq_ctx->data.ifOperStatus;
    return MFD_SUCCESS;
}

int ifLastChange_get(ifTable_rowreq_ctx *rowreq_ctx,
                     long *ifLastChange_val_ptr) {
    netsnmp_assert(NULL != ifLastChange_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifLastChange_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifLastChange_val_ptr) = rowreq_ctx->data.ifLastChange;
    return MFD_SUCCESS;
}

int ifInOctets_get(ifTable_rowreq_ctx *rowreq_ctx, u_long *ifInOctets_val_ptr) {
    netsnmp_assert(NULL != ifInOctets_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifInOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifInOctets_val_ptr) = rowreq_ctx->data.ifInOctets;
    return MFD_SUCCESS;
}

int ifInUcastPkts_get(ifTable_rowreq_ctx *rowreq_ctx,
                      u_long *ifInUcastPkts_val_ptr) {
    netsnmp_assert(NULL != ifInUcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifInUcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifInUcastPkts_val_ptr) = rowreq_ctx->data.ifInUcastPkts;
    return MFD_SUCCESS;
}

int ifInNUcastPkts_get(ifTable_rowreq_ctx *rowreq_ctx,
                       u_long *ifInNUcastPkts_val_ptr) {
    netsnmp_assert(NULL != ifInNUcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifInNUcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifInNUcastPkts_val_ptr) = rowreq_ctx->data.ifInNUcastPkts;
    return MFD_SUCCESS;
}

int ifInDiscards_get(ifTable_rowreq_ctx *rowreq_ctx,
                     u_long *ifInDiscards_val_ptr) {
    netsnmp_assert(NULL != ifInDiscards_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifInDiscards_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifInDiscards_val_ptr) = rowreq_ctx->data.ifInDiscards;
    return MFD_SUCCESS;
}

int ifInErrors_get(ifTable_rowreq_ctx *rowreq_ctx, u_long *ifInErrors_val_ptr) {
    netsnmp_assert(NULL != ifInErrors_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifInErrors_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifInErrors_val_ptr) = rowreq_ctx->data.ifInErrors;
    return MFD_SUCCESS;
}

int ifInUnknownProtos_get(ifTable_rowreq_ctx *rowreq_ctx,
                          u_long *ifInUnknownProtos_val_ptr) {
    netsnmp_assert(NULL != ifInUnknownProtos_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifInUnknownProtos_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifInUnknownProtos_val_ptr) = rowreq_ctx->data.ifInUnknownProtos;
    return MFD_SUCCESS;
}

int ifOutOctets_get(ifTable_rowreq_ctx *rowreq_ctx,
                    u_long *ifOutOctets_val_ptr) {
    netsnmp_assert(NULL != ifOutOctets_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifOutOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifOutOctets_val_ptr) = rowreq_ctx->data.ifOutOctets;
    return MFD_SUCCESS;
}

int ifOutUcastPkts_get(ifTable_rowreq_ctx *rowreq_ctx,
                       u_long *ifOutUcastPkts_val_ptr) {
    netsnmp_assert(NULL != ifOutUcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifOutUcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifOutUcastPkts_val_ptr) = rowreq_ctx->data.ifOutUcastPkts;
    return MFD_SUCCESS;
}

int ifOutNUcastPkts_get(ifTable_rowreq_ctx *rowreq_ctx,
                        u_long *ifOutNUcastPkts_val_ptr) {
    netsnmp_assert(NULL != ifOutNUcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifOutNUcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifOutNUcastPkts_val_ptr) = rowreq_ctx->data.ifOutNUcastPkts;
    return MFD_SUCCESS;
}

int ifOutDiscards_get(ifTable_rowreq_ctx *rowreq_ctx,
                      u_long *ifOutDiscards_val_ptr) {
    netsnmp_assert(NULL != ifOutDiscards_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifOutDiscards_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifOutDiscards_val_ptr) = rowreq_ctx->data.ifOutDiscards;
    return MFD_SUCCESS;
}

int ifOutErrors_get(ifTable_rowreq_ctx *rowreq_ctx,
                    u_long *ifOutErrors_val_ptr) {
    netsnmp_assert(NULL != ifOutErrors_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifOutErrors_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifOutErrors_val_ptr) = rowreq_ctx->data.ifOutErrors;
    return MFD_SUCCESS;
}

int ifOutQLen_get(ifTable_rowreq_ctx *rowreq_ctx, u_long *ifOutQLen_val_ptr) {
    netsnmp_assert(NULL != ifOutQLen_val_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifOutQLen_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifOutQLen_val_ptr) = rowreq_ctx->data.ifOutQLen;
    return MFD_SUCCESS;
}

int ifSpecific_get(ifTable_rowreq_ctx *rowreq_ctx, oid **ifSpecific_val_ptr_ptr,
                   size_t *ifSpecific_val_ptr_len_ptr) {
    netsnmp_assert((NULL != ifSpecific_val_ptr_ptr) &&
                   (NULL != *ifSpecific_val_ptr_ptr));
    netsnmp_assert(NULL != ifSpecific_val_ptr_len_ptr);
    DEBUGMSGTL(("verbose:ifTable:ifSpecific_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    if ((NULL == (*ifSpecific_val_ptr_ptr)) ||
        ((*ifSpecific_val_ptr_len_ptr) <
         (rowreq_ctx->data.ifSpecific_len *
          sizeof(rowreq_ctx->data.ifSpecific[0])))) {
        (*ifSpecific_val_ptr_ptr) =
            malloc(rowreq_ctx->data.ifSpecific_len *
                   sizeof(rowreq_ctx->data.ifSpecific[0]));
        if (NULL == (*ifSpecific_val_ptr_ptr)) {
            snmp_log(
                LOG_ERR,
                "could not allocate memory (rowreq_ctx->data.ifSpecific)\n");
            return MFD_ERROR;
        }
    }
    (*ifSpecific_val_ptr_len_ptr) = rowreq_ctx->data.ifSpecific_len *
                                    sizeof(rowreq_ctx->data.ifSpecific[0]);
    memcpy((*ifSpecific_val_ptr_ptr), rowreq_ctx->data.ifSpecific,
           rowreq_ctx->data.ifSpecific_len *
               sizeof(rowreq_ctx->data.ifSpecific[0]));
    return MFD_SUCCESS;
}
