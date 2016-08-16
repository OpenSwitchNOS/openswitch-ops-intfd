#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "ifXTable.h"

int ifXTable_indexes_set_tbl_idx(ifXTable_mib_index *tbl_idx,
                                 long ifIndex_val) {
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_indexes_set_tbl_idx", "called\n"));

    tbl_idx->ifIndex = ifIndex_val;
    return MFD_SUCCESS;
}

int ifXTable_indexes_set(ifXTable_rowreq_ctx *rowreq_ctx, long ifIndex_val) {
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_indexes_set", "called\n"));
    if (MFD_SUCCESS !=
        ifXTable_indexes_set_tbl_idx(&rowreq_ctx->tbl_idx, ifIndex_val)) {
        return MFD_ERROR;
    }
    rowreq_ctx->oid_idx.len = sizeof(rowreq_ctx->oid_tmp) / sizeof(oid);
    if (0 !=
        ifXTable_index_to_oid(&rowreq_ctx->oid_idx, &rowreq_ctx->tbl_idx)) {
        return MFD_ERROR;
    }
    return MFD_SUCCESS;
}

int ifName_get(ifXTable_rowreq_ctx *rowreq_ctx, char **ifName_val_ptr_ptr,
               size_t *ifName_val_ptr_len_ptr) {
    netsnmp_assert((NULL != ifName_val_ptr_ptr) &&
                   (NULL != *ifName_val_ptr_ptr));
    netsnmp_assert(NULL != ifName_val_ptr_len_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifName_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    if ((NULL == (*ifName_val_ptr_ptr)) ||
        ((*ifName_val_ptr_len_ptr) <
         (rowreq_ctx->data.ifName_len * sizeof(rowreq_ctx->data.ifName[0])))) {
        (*ifName_val_ptr_ptr) = malloc(rowreq_ctx->data.ifName_len *
                                       sizeof(rowreq_ctx->data.ifName[0]));
        if (NULL == (*ifName_val_ptr_ptr)) {
            snmp_log(LOG_ERR,
                     "could not allocate memory (rowreq_ctx->data.ifName)\n");
            return MFD_ERROR;
        }
    }
    (*ifName_val_ptr_len_ptr) =
        rowreq_ctx->data.ifName_len * sizeof(rowreq_ctx->data.ifName[0]);
    memcpy((*ifName_val_ptr_ptr), rowreq_ctx->data.ifName,
           rowreq_ctx->data.ifName_len * sizeof(rowreq_ctx->data.ifName[0]));
    return MFD_SUCCESS;
}

int ifInMulticastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                          u_long *ifInMulticastPkts_val_ptr) {
    netsnmp_assert(NULL != ifInMulticastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifInMulticastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifInMulticastPkts_val_ptr) = rowreq_ctx->data.ifInMulticastPkts;
    return MFD_SUCCESS;
}

int ifInBroadcastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                          u_long *ifInBroadcastPkts_val_ptr) {
    netsnmp_assert(NULL != ifInBroadcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifInBroadcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifInBroadcastPkts_val_ptr) = rowreq_ctx->data.ifInBroadcastPkts;
    return MFD_SUCCESS;
}

int ifOutMulticastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                           u_long *ifOutMulticastPkts_val_ptr) {
    netsnmp_assert(NULL != ifOutMulticastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifOutMulticastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifOutMulticastPkts_val_ptr) = rowreq_ctx->data.ifOutMulticastPkts;
    return MFD_SUCCESS;
}

int ifOutBroadcastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                           u_long *ifOutBroadcastPkts_val_ptr) {
    netsnmp_assert(NULL != ifOutBroadcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifOutBroadcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifOutBroadcastPkts_val_ptr) = rowreq_ctx->data.ifOutBroadcastPkts;
    return MFD_SUCCESS;
}

int ifHCInOctets_get(ifXTable_rowreq_ctx *rowreq_ctx,
                     U64 *ifHCInOctets_val_ptr) {
    netsnmp_assert(NULL != ifHCInOctets_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifHCInOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifHCInOctets_val_ptr) = rowreq_ctx->data.ifHCInOctets;
    return MFD_SUCCESS;
}

int ifHCInUcastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                        U64 *ifHCInUcastPkts_val_ptr) {
    netsnmp_assert(NULL != ifHCInUcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifHCInUcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifHCInUcastPkts_val_ptr) = rowreq_ctx->data.ifHCInUcastPkts;
    return MFD_SUCCESS;
}

int ifHCInMulticastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                            U64 *ifHCInMulticastPkts_val_ptr) {
    netsnmp_assert(NULL != ifHCInMulticastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifHCInMulticastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifHCInMulticastPkts_val_ptr) = rowreq_ctx->data.ifHCInMulticastPkts;
    return MFD_SUCCESS;
}

int ifHCInBroadcastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                            U64 *ifHCInBroadcastPkts_val_ptr) {
    netsnmp_assert(NULL != ifHCInBroadcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifHCInBroadcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifHCInBroadcastPkts_val_ptr) = rowreq_ctx->data.ifHCInBroadcastPkts;
    return MFD_SUCCESS;
}

int ifHCOutOctets_get(ifXTable_rowreq_ctx *rowreq_ctx,
                      U64 *ifHCOutOctets_val_ptr) {
    netsnmp_assert(NULL != ifHCOutOctets_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifHCOutOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifHCOutOctets_val_ptr) = rowreq_ctx->data.ifHCOutOctets;
    return MFD_SUCCESS;
}

int ifHCOutUcastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                         U64 *ifHCOutUcastPkts_val_ptr) {
    netsnmp_assert(NULL != ifHCOutUcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifHCOutUcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifHCOutUcastPkts_val_ptr) = rowreq_ctx->data.ifHCOutUcastPkts;
    return MFD_SUCCESS;
}

int ifHCOutMulticastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                             U64 *ifHCOutMulticastPkts_val_ptr) {
    netsnmp_assert(NULL != ifHCOutMulticastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifHCOutMulticastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifHCOutMulticastPkts_val_ptr) = rowreq_ctx->data.ifHCOutMulticastPkts;
    return MFD_SUCCESS;
}

int ifHCOutBroadcastPkts_get(ifXTable_rowreq_ctx *rowreq_ctx,
                             U64 *ifHCOutBroadcastPkts_val_ptr) {
    netsnmp_assert(NULL != ifHCOutBroadcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifHCOutBroadcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifHCOutBroadcastPkts_val_ptr) = rowreq_ctx->data.ifHCOutBroadcastPkts;
    return MFD_SUCCESS;
}

int ifLinkUpDownTrapEnable_get(ifXTable_rowreq_ctx *rowreq_ctx,
                               long *ifLinkUpDownTrapEnable_val_ptr) {
    netsnmp_assert(NULL != ifLinkUpDownTrapEnable_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifLinkUpDownTrapEnable_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifLinkUpDownTrapEnable_val_ptr) = rowreq_ctx->data.ifLinkUpDownTrapEnable;
    return MFD_SUCCESS;
}

int ifHighSpeed_get(ifXTable_rowreq_ctx *rowreq_ctx,
                    u_long *ifHighSpeed_val_ptr) {
    netsnmp_assert(NULL != ifHighSpeed_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifHighSpeed_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifHighSpeed_val_ptr) = rowreq_ctx->data.ifHighSpeed;
    return MFD_SUCCESS;
}

int ifPromiscuousMode_get(ifXTable_rowreq_ctx *rowreq_ctx,
                          long *ifPromiscuousMode_val_ptr) {
    netsnmp_assert(NULL != ifPromiscuousMode_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifPromiscuousMode_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifPromiscuousMode_val_ptr) = rowreq_ctx->data.ifPromiscuousMode;
    return MFD_SUCCESS;
}

int ifConnectorPresent_get(ifXTable_rowreq_ctx *rowreq_ctx,
                           long *ifConnectorPresent_val_ptr) {
    netsnmp_assert(NULL != ifConnectorPresent_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifConnectorPresent_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifConnectorPresent_val_ptr) = rowreq_ctx->data.ifConnectorPresent;
    return MFD_SUCCESS;
}

int ifAlias_get(ifXTable_rowreq_ctx *rowreq_ctx, char **ifAlias_val_ptr_ptr,
                size_t *ifAlias_val_ptr_len_ptr) {
    netsnmp_assert((NULL != ifAlias_val_ptr_ptr) &&
                   (NULL != *ifAlias_val_ptr_ptr));
    netsnmp_assert(NULL != ifAlias_val_ptr_len_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifAlias_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    if ((NULL == (*ifAlias_val_ptr_ptr)) ||
        ((*ifAlias_val_ptr_len_ptr) < (rowreq_ctx->data.ifAlias_len *
                                       sizeof(rowreq_ctx->data.ifAlias[0])))) {
        (*ifAlias_val_ptr_ptr) = malloc(rowreq_ctx->data.ifAlias_len *
                                        sizeof(rowreq_ctx->data.ifAlias[0]));
        if (NULL == (*ifAlias_val_ptr_ptr)) {
            snmp_log(LOG_ERR,
                     "could not allocate memory (rowreq_ctx->data.ifAlias)\n");
            return MFD_ERROR;
        }
    }
    (*ifAlias_val_ptr_len_ptr) =
        rowreq_ctx->data.ifAlias_len * sizeof(rowreq_ctx->data.ifAlias[0]);
    memcpy((*ifAlias_val_ptr_ptr), rowreq_ctx->data.ifAlias,
           rowreq_ctx->data.ifAlias_len * sizeof(rowreq_ctx->data.ifAlias[0]));
    return MFD_SUCCESS;
}

int ifCounterDiscontinuityTime_get(ifXTable_rowreq_ctx *rowreq_ctx,
                                   long *ifCounterDiscontinuityTime_val_ptr) {
    netsnmp_assert(NULL != ifCounterDiscontinuityTime_val_ptr);
    DEBUGMSGTL(("verbose:ifXTable:ifCounterDiscontinuityTime_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ifCounterDiscontinuityTime_val_ptr) =
        rowreq_ctx->data.ifCounterDiscontinuityTime;
    return MFD_SUCCESS;
}
