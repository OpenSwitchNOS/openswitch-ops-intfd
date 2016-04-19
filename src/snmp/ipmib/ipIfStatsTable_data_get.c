#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "ipIfStatsTable.h"

int ipIfStatsTable_indexes_set_tbl_idx(ipIfStatsTable_mib_index *tbl_idx,
                                       long ipIfStatsIPVersion_val,
                                       long ipIfStatsIfIndex_val) {
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsTable_indexes_set_tbl_idx",
                "called\n"));

    tbl_idx->ipIfStatsIPVersion = ipIfStatsIPVersion_val;
    tbl_idx->ipIfStatsIfIndex = ipIfStatsIfIndex_val;
    return MFD_SUCCESS;
}

int ipIfStatsTable_indexes_set(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                               long ipIfStatsIPVersion_val,
                               long ipIfStatsIfIndex_val) {
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsTable_indexes_set", "called\n"));
    if (MFD_SUCCESS != ipIfStatsTable_indexes_set_tbl_idx(
                           &rowreq_ctx->tbl_idx, ipIfStatsIPVersion_val,
                           ipIfStatsIfIndex_val)) {
        return MFD_ERROR;
    }
    rowreq_ctx->oid_idx.len = sizeof(rowreq_ctx->oid_tmp) / sizeof(oid);
    if (0 != ipIfStatsTable_index_to_oid(&rowreq_ctx->oid_idx,
                                         &rowreq_ctx->tbl_idx)) {
        return MFD_ERROR;
    }
    return MFD_SUCCESS;
}

int ipIfStatsInReceives_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsInReceives_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInReceives_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsInReceives_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInReceives_val_ptr) = rowreq_ctx->data.ipIfStatsInReceives;
    return MFD_SUCCESS;
}

int ipIfStatsHCInReceives_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCInReceives_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCInReceives_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsHCInReceives_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCInReceives_val_ptr) = rowreq_ctx->data.ipIfStatsHCInReceives;
    return MFD_SUCCESS;
}

int ipIfStatsInOctets_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                          u_long *ipIfStatsInOctets_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInOctets_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsInOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInOctets_val_ptr) = rowreq_ctx->data.ipIfStatsInOctets;
    return MFD_SUCCESS;
}

int ipIfStatsHCInOctets_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            unsigned long long *ipIfStatsHCInOctets_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCInOctets_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsHCInOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCInOctets_val_ptr) = rowreq_ctx->data.ipIfStatsHCInOctets;
    return MFD_SUCCESS;
}

int ipIfStatsInHdrErrors_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             u_long *ipIfStatsInHdrErrors_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInHdrErrors_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsInHdrErrors_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInHdrErrors_val_ptr) = rowreq_ctx->data.ipIfStatsInHdrErrors;
    return MFD_SUCCESS;
}

int ipIfStatsInNoRoutes_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsInNoRoutes_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInNoRoutes_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsInNoRoutes_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInNoRoutes_val_ptr) = rowreq_ctx->data.ipIfStatsInNoRoutes;
    return MFD_SUCCESS;
}

int ipIfStatsInAddrErrors_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipIfStatsInAddrErrors_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInAddrErrors_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsInAddrErrors_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInAddrErrors_val_ptr) = rowreq_ctx->data.ipIfStatsInAddrErrors;
    return MFD_SUCCESS;
}

int ipIfStatsInUnknownProtos_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipIfStatsInUnknownProtos_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInUnknownProtos_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsInUnknownProtos_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInUnknownProtos_val_ptr) =
        rowreq_ctx->data.ipIfStatsInUnknownProtos;
    return MFD_SUCCESS;
}

int ipIfStatsInTruncatedPkts_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipIfStatsInTruncatedPkts_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInTruncatedPkts_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsInTruncatedPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInTruncatedPkts_val_ptr) =
        rowreq_ctx->data.ipIfStatsInTruncatedPkts;
    return MFD_SUCCESS;
}

int ipIfStatsInForwDatagrams_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipIfStatsInForwDatagrams_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInForwDatagrams_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsInForwDatagrams_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInForwDatagrams_val_ptr) =
        rowreq_ctx->data.ipIfStatsInForwDatagrams;
    return MFD_SUCCESS;
}

int ipIfStatsHCInForwDatagrams_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCInForwDatagrams_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCInForwDatagrams_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsHCInForwDatagrams_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCInForwDatagrams_val_ptr) =
        rowreq_ctx->data.ipIfStatsHCInForwDatagrams;
    return MFD_SUCCESS;
}

int ipIfStatsReasmReqds_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsReasmReqds_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsReasmReqds_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsReasmReqds_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsReasmReqds_val_ptr) = rowreq_ctx->data.ipIfStatsReasmReqds;
    return MFD_SUCCESS;
}

int ipIfStatsReasmOKs_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                          u_long *ipIfStatsReasmOKs_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsReasmOKs_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsReasmOKs_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsReasmOKs_val_ptr) = rowreq_ctx->data.ipIfStatsReasmOKs;
    return MFD_SUCCESS;
}

int ipIfStatsReasmFails_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsReasmFails_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsReasmFails_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsReasmFails_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsReasmFails_val_ptr) = rowreq_ctx->data.ipIfStatsReasmFails;
    return MFD_SUCCESS;
}

int ipIfStatsInDiscards_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsInDiscards_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInDiscards_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsInDiscards_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInDiscards_val_ptr) = rowreq_ctx->data.ipIfStatsInDiscards;
    return MFD_SUCCESS;
}

int ipIfStatsInDelivers_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsInDelivers_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInDelivers_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsInDelivers_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInDelivers_val_ptr) = rowreq_ctx->data.ipIfStatsInDelivers;
    return MFD_SUCCESS;
}

int ipIfStatsHCInDelivers_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCInDelivers_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCInDelivers_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsHCInDelivers_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCInDelivers_val_ptr) = rowreq_ctx->data.ipIfStatsHCInDelivers;
    return MFD_SUCCESS;
}

int ipIfStatsOutRequests_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             u_long *ipIfStatsOutRequests_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsOutRequests_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsOutRequests_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsOutRequests_val_ptr) = rowreq_ctx->data.ipIfStatsOutRequests;
    return MFD_SUCCESS;
}

int ipIfStatsHCOutRequests_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCOutRequests_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCOutRequests_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsHCOutRequests_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCOutRequests_val_ptr) = rowreq_ctx->data.ipIfStatsHCOutRequests;
    return MFD_SUCCESS;
}

int ipIfStatsOutForwDatagrams_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipIfStatsOutForwDatagrams_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsOutForwDatagrams_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsOutForwDatagrams_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsOutForwDatagrams_val_ptr) =
        rowreq_ctx->data.ipIfStatsOutForwDatagrams;
    return MFD_SUCCESS;
}

int ipIfStatsHCOutForwDatagrams_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCOutForwDatagrams_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCOutForwDatagrams_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsHCOutForwDatagrams_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCOutForwDatagrams_val_ptr) =
        rowreq_ctx->data.ipIfStatsHCOutForwDatagrams;
    return MFD_SUCCESS;
}

int ipIfStatsOutDiscards_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             u_long *ipIfStatsOutDiscards_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsOutDiscards_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsOutDiscards_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsOutDiscards_val_ptr) = rowreq_ctx->data.ipIfStatsOutDiscards;
    return MFD_SUCCESS;
}

int ipIfStatsOutFragReqds_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipIfStatsOutFragReqds_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsOutFragReqds_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsOutFragReqds_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsOutFragReqds_val_ptr) = rowreq_ctx->data.ipIfStatsOutFragReqds;
    return MFD_SUCCESS;
}

int ipIfStatsOutFragOKs_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                            u_long *ipIfStatsOutFragOKs_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsOutFragOKs_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsOutFragOKs_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsOutFragOKs_val_ptr) = rowreq_ctx->data.ipIfStatsOutFragOKs;
    return MFD_SUCCESS;
}

int ipIfStatsOutFragFails_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipIfStatsOutFragFails_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsOutFragFails_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsOutFragFails_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsOutFragFails_val_ptr) = rowreq_ctx->data.ipIfStatsOutFragFails;
    return MFD_SUCCESS;
}

int ipIfStatsOutFragCreates_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipIfStatsOutFragCreates_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsOutFragCreates_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsOutFragCreates_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsOutFragCreates_val_ptr) =
        rowreq_ctx->data.ipIfStatsOutFragCreates;
    return MFD_SUCCESS;
}

int ipIfStatsOutTransmits_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipIfStatsOutTransmits_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsOutTransmits_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsOutTransmits_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsOutTransmits_val_ptr) = rowreq_ctx->data.ipIfStatsOutTransmits;
    return MFD_SUCCESS;
}

int ipIfStatsHCOutTransmits_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCOutTransmits_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCOutTransmits_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsHCOutTransmits_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCOutTransmits_val_ptr) =
        rowreq_ctx->data.ipIfStatsHCOutTransmits;
    return MFD_SUCCESS;
}

int ipIfStatsOutOctets_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                           u_long *ipIfStatsOutOctets_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsOutOctets_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsOutOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsOutOctets_val_ptr) = rowreq_ctx->data.ipIfStatsOutOctets;
    return MFD_SUCCESS;
}

int ipIfStatsHCOutOctets_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             unsigned long long *ipIfStatsHCOutOctets_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCOutOctets_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsHCOutOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCOutOctets_val_ptr) = rowreq_ctx->data.ipIfStatsHCOutOctets;
    return MFD_SUCCESS;
}

int ipIfStatsInMcastPkts_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             u_long *ipIfStatsInMcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInMcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsInMcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInMcastPkts_val_ptr) = rowreq_ctx->data.ipIfStatsInMcastPkts;
    return MFD_SUCCESS;
}

int ipIfStatsHCInMcastPkts_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCInMcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCInMcastPkts_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsHCInMcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCInMcastPkts_val_ptr) = rowreq_ctx->data.ipIfStatsHCInMcastPkts;
    return MFD_SUCCESS;
}

int ipIfStatsInMcastOctets_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                               u_long *ipIfStatsInMcastOctets_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInMcastOctets_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsInMcastOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInMcastOctets_val_ptr) = rowreq_ctx->data.ipIfStatsInMcastOctets;
    return MFD_SUCCESS;
}

int ipIfStatsHCInMcastOctets_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCInMcastOctets_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCInMcastOctets_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsHCInMcastOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCInMcastOctets_val_ptr) =
        rowreq_ctx->data.ipIfStatsHCInMcastOctets;
    return MFD_SUCCESS;
}

int ipIfStatsOutMcastPkts_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipIfStatsOutMcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsOutMcastPkts_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsOutMcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsOutMcastPkts_val_ptr) = rowreq_ctx->data.ipIfStatsOutMcastPkts;
    return MFD_SUCCESS;
}

int ipIfStatsHCOutMcastPkts_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCOutMcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCOutMcastPkts_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsHCOutMcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCOutMcastPkts_val_ptr) =
        rowreq_ctx->data.ipIfStatsHCOutMcastPkts;
    return MFD_SUCCESS;
}

int ipIfStatsOutMcastOctets_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipIfStatsOutMcastOctets_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsOutMcastOctets_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsOutMcastOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsOutMcastOctets_val_ptr) =
        rowreq_ctx->data.ipIfStatsOutMcastOctets;
    return MFD_SUCCESS;
}

int ipIfStatsHCOutMcastOctets_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCOutMcastOctets_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCOutMcastOctets_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsHCOutMcastOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCOutMcastOctets_val_ptr) =
        rowreq_ctx->data.ipIfStatsHCOutMcastOctets;
    return MFD_SUCCESS;
}

int ipIfStatsInBcastPkts_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             u_long *ipIfStatsInBcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsInBcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsInBcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsInBcastPkts_val_ptr) = rowreq_ctx->data.ipIfStatsInBcastPkts;
    return MFD_SUCCESS;
}

int ipIfStatsHCInBcastPkts_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCInBcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCInBcastPkts_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsHCInBcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCInBcastPkts_val_ptr) = rowreq_ctx->data.ipIfStatsHCInBcastPkts;
    return MFD_SUCCESS;
}

int ipIfStatsOutBcastPkts_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipIfStatsOutBcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsOutBcastPkts_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsOutBcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsOutBcastPkts_val_ptr) = rowreq_ctx->data.ipIfStatsOutBcastPkts;
    return MFD_SUCCESS;
}

int ipIfStatsHCOutBcastPkts_get(
    ipIfStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipIfStatsHCOutBcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsHCOutBcastPkts_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsHCOutBcastPkts_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsHCOutBcastPkts_val_ptr) =
        rowreq_ctx->data.ipIfStatsHCOutBcastPkts;
    return MFD_SUCCESS;
}

int ipIfStatsDiscontinuityTime_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                   long *ipIfStatsDiscontinuityTime_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsDiscontinuityTime_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsDiscontinuityTime_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsDiscontinuityTime_val_ptr) =
        rowreq_ctx->data.ipIfStatsDiscontinuityTime;
    return MFD_SUCCESS;
}

int ipIfStatsRefreshRate_get(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                             u_long *ipIfStatsRefreshRate_val_ptr) {
    netsnmp_assert(NULL != ipIfStatsRefreshRate_val_ptr);
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsRefreshRate_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipIfStatsRefreshRate_val_ptr) = rowreq_ctx->data.ipIfStatsRefreshRate;
    return MFD_SUCCESS;
}
