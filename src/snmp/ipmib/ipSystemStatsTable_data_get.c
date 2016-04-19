#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "ipSystemStatsTable.h"

int ipSystemStatsTable_indexes_set_tbl_idx(
    ipSystemStatsTable_mib_index *tbl_idx, long ipSystemStatsIPVersion_val) {
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsTable_indexes_set_tbl_idx",
         "called\n"));

    tbl_idx->ipSystemStatsIPVersion = ipSystemStatsIPVersion_val;
    return MFD_SUCCESS;
}

int ipSystemStatsTable_indexes_set(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                   long ipSystemStatsIPVersion_val) {
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsTable_indexes_set",
                "called\n"));
    if (MFD_SUCCESS != ipSystemStatsTable_indexes_set_tbl_idx(
                           &rowreq_ctx->tbl_idx, ipSystemStatsIPVersion_val)) {
        return MFD_ERROR;
    }
    rowreq_ctx->oid_idx.len = sizeof(rowreq_ctx->oid_tmp) / sizeof(oid);
    if (0 != ipSystemStatsTable_index_to_oid(&rowreq_ctx->oid_idx,
                                             &rowreq_ctx->tbl_idx)) {
        return MFD_ERROR;
    }
    return MFD_SUCCESS;
}

int ipSystemStatsInReceives_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsInReceives_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInReceives_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsInReceives_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInReceives_val_ptr) =
        rowreq_ctx->data.ipSystemStatsInReceives;
    return MFD_SUCCESS;
}

int ipSystemStatsHCInReceives_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInReceives_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCInReceives_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsHCInReceives_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCInReceives_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCInReceives;
    return MFD_SUCCESS;
}

int ipSystemStatsInOctets_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipSystemStatsInOctets_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInOctets_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsInOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInOctets_val_ptr) = rowreq_ctx->data.ipSystemStatsInOctets;
    return MFD_SUCCESS;
}

int ipSystemStatsHCInOctets_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInOctets_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCInOctets_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsHCInOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCInOctets_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCInOctets;
    return MFD_SUCCESS;
}

int ipSystemStatsInHdrErrors_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsInHdrErrors_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInHdrErrors_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsInHdrErrors_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInHdrErrors_val_ptr) =
        rowreq_ctx->data.ipSystemStatsInHdrErrors;
    return MFD_SUCCESS;
}

int ipSystemStatsInNoRoutes_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsInNoRoutes_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInNoRoutes_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsInNoRoutes_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInNoRoutes_val_ptr) =
        rowreq_ctx->data.ipSystemStatsInNoRoutes;
    return MFD_SUCCESS;
}

int ipSystemStatsInAddrErrors_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipSystemStatsInAddrErrors_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInAddrErrors_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsInAddrErrors_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInAddrErrors_val_ptr) =
        rowreq_ctx->data.ipSystemStatsInAddrErrors;
    return MFD_SUCCESS;
}

int ipSystemStatsInUnknownProtos_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    u_long *ipSystemStatsInUnknownProtos_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInUnknownProtos_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsInUnknownProtos_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInUnknownProtos_val_ptr) =
        rowreq_ctx->data.ipSystemStatsInUnknownProtos;
    return MFD_SUCCESS;
}

int ipSystemStatsInTruncatedPkts_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    u_long *ipSystemStatsInTruncatedPkts_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInTruncatedPkts_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsInTruncatedPkts_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInTruncatedPkts_val_ptr) =
        rowreq_ctx->data.ipSystemStatsInTruncatedPkts;
    return MFD_SUCCESS;
}

int ipSystemStatsInForwDatagrams_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    u_long *ipSystemStatsInForwDatagrams_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInForwDatagrams_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsInForwDatagrams_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInForwDatagrams_val_ptr) =
        rowreq_ctx->data.ipSystemStatsInForwDatagrams;
    return MFD_SUCCESS;
}

int ipSystemStatsHCInForwDatagrams_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInForwDatagrams_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCInForwDatagrams_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsHCInForwDatagrams_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCInForwDatagrams_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCInForwDatagrams;
    return MFD_SUCCESS;
}

int ipSystemStatsReasmReqds_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsReasmReqds_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsReasmReqds_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsReasmReqds_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsReasmReqds_val_ptr) =
        rowreq_ctx->data.ipSystemStatsReasmReqds;
    return MFD_SUCCESS;
}

int ipSystemStatsReasmOKs_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                              u_long *ipSystemStatsReasmOKs_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsReasmOKs_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsReasmOKs_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsReasmOKs_val_ptr) = rowreq_ctx->data.ipSystemStatsReasmOKs;
    return MFD_SUCCESS;
}

int ipSystemStatsReasmFails_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsReasmFails_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsReasmFails_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsReasmFails_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsReasmFails_val_ptr) =
        rowreq_ctx->data.ipSystemStatsReasmFails;
    return MFD_SUCCESS;
}

int ipSystemStatsInDiscards_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsInDiscards_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInDiscards_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsInDiscards_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInDiscards_val_ptr) =
        rowreq_ctx->data.ipSystemStatsInDiscards;
    return MFD_SUCCESS;
}

int ipSystemStatsInDelivers_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsInDelivers_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInDelivers_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsInDelivers_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInDelivers_val_ptr) =
        rowreq_ctx->data.ipSystemStatsInDelivers;
    return MFD_SUCCESS;
}

int ipSystemStatsHCInDelivers_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInDelivers_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCInDelivers_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsHCInDelivers_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCInDelivers_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCInDelivers;
    return MFD_SUCCESS;
}

int ipSystemStatsOutRequests_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsOutRequests_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutRequests_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsOutRequests_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutRequests_val_ptr) =
        rowreq_ctx->data.ipSystemStatsOutRequests;
    return MFD_SUCCESS;
}

int ipSystemStatsHCOutRequests_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutRequests_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCOutRequests_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsHCOutRequests_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCOutRequests_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCOutRequests;
    return MFD_SUCCESS;
}

int ipSystemStatsOutNoRoutes_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsOutNoRoutes_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutNoRoutes_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsOutNoRoutes_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutNoRoutes_val_ptr) =
        rowreq_ctx->data.ipSystemStatsOutNoRoutes;
    return MFD_SUCCESS;
}

int ipSystemStatsOutForwDatagrams_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    u_long *ipSystemStatsOutForwDatagrams_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutForwDatagrams_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsOutForwDatagrams_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutForwDatagrams_val_ptr) =
        rowreq_ctx->data.ipSystemStatsOutForwDatagrams;
    return MFD_SUCCESS;
}

int ipSystemStatsHCOutForwDatagrams_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutForwDatagrams_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCOutForwDatagrams_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsHCOutForwDatagrams_get",
         "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCOutForwDatagrams_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCOutForwDatagrams;
    return MFD_SUCCESS;
}

int ipSystemStatsOutDiscards_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsOutDiscards_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutDiscards_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsOutDiscards_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutDiscards_val_ptr) =
        rowreq_ctx->data.ipSystemStatsOutDiscards;
    return MFD_SUCCESS;
}

int ipSystemStatsOutFragReqds_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipSystemStatsOutFragReqds_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutFragReqds_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsOutFragReqds_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutFragReqds_val_ptr) =
        rowreq_ctx->data.ipSystemStatsOutFragReqds;
    return MFD_SUCCESS;
}

int ipSystemStatsOutFragOKs_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                u_long *ipSystemStatsOutFragOKs_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutFragOKs_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsOutFragOKs_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutFragOKs_val_ptr) =
        rowreq_ctx->data.ipSystemStatsOutFragOKs;
    return MFD_SUCCESS;
}

int ipSystemStatsOutFragFails_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipSystemStatsOutFragFails_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutFragFails_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsOutFragFails_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutFragFails_val_ptr) =
        rowreq_ctx->data.ipSystemStatsOutFragFails;
    return MFD_SUCCESS;
}

int ipSystemStatsOutFragCreates_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    u_long *ipSystemStatsOutFragCreates_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutFragCreates_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsOutFragCreates_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutFragCreates_val_ptr) =
        rowreq_ctx->data.ipSystemStatsOutFragCreates;
    return MFD_SUCCESS;
}

int ipSystemStatsOutTransmits_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipSystemStatsOutTransmits_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutTransmits_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsOutTransmits_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutTransmits_val_ptr) =
        rowreq_ctx->data.ipSystemStatsOutTransmits;
    return MFD_SUCCESS;
}

int ipSystemStatsHCOutTransmits_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutTransmits_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCOutTransmits_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsHCOutTransmits_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCOutTransmits_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCOutTransmits;
    return MFD_SUCCESS;
}

int ipSystemStatsOutOctets_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                               u_long *ipSystemStatsOutOctets_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutOctets_val_ptr);
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsOutOctets_get", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutOctets_val_ptr) = rowreq_ctx->data.ipSystemStatsOutOctets;
    return MFD_SUCCESS;
}

int ipSystemStatsHCOutOctets_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutOctets_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCOutOctets_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsHCOutOctets_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCOutOctets_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCOutOctets;
    return MFD_SUCCESS;
}

int ipSystemStatsInMcastPkts_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsInMcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInMcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsInMcastPkts_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInMcastPkts_val_ptr) =
        rowreq_ctx->data.ipSystemStatsInMcastPkts;
    return MFD_SUCCESS;
}

int ipSystemStatsHCInMcastPkts_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInMcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCInMcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsHCInMcastPkts_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCInMcastPkts_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCInMcastPkts;
    return MFD_SUCCESS;
}

int ipSystemStatsInMcastOctets_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                   u_long *ipSystemStatsInMcastOctets_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInMcastOctets_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsInMcastOctets_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInMcastOctets_val_ptr) =
        rowreq_ctx->data.ipSystemStatsInMcastOctets;
    return MFD_SUCCESS;
}

int ipSystemStatsHCInMcastOctets_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInMcastOctets_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCInMcastOctets_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsHCInMcastOctets_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCInMcastOctets_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCInMcastOctets;
    return MFD_SUCCESS;
}

int ipSystemStatsOutMcastPkts_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipSystemStatsOutMcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutMcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsOutMcastPkts_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutMcastPkts_val_ptr) =
        rowreq_ctx->data.ipSystemStatsOutMcastPkts;
    return MFD_SUCCESS;
}

int ipSystemStatsHCOutMcastPkts_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutMcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCOutMcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsHCOutMcastPkts_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCOutMcastPkts_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCOutMcastPkts;
    return MFD_SUCCESS;
}

int ipSystemStatsOutMcastOctets_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    u_long *ipSystemStatsOutMcastOctets_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutMcastOctets_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsOutMcastOctets_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutMcastOctets_val_ptr) =
        rowreq_ctx->data.ipSystemStatsOutMcastOctets;
    return MFD_SUCCESS;
}

int ipSystemStatsHCOutMcastOctets_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutMcastOctets_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCOutMcastOctets_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsHCOutMcastOctets_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCOutMcastOctets_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCOutMcastOctets;
    return MFD_SUCCESS;
}

int ipSystemStatsInBcastPkts_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsInBcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsInBcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsInBcastPkts_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsInBcastPkts_val_ptr) =
        rowreq_ctx->data.ipSystemStatsInBcastPkts;
    return MFD_SUCCESS;
}

int ipSystemStatsHCInBcastPkts_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCInBcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCInBcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsHCInBcastPkts_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCInBcastPkts_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCInBcastPkts;
    return MFD_SUCCESS;
}

int ipSystemStatsOutBcastPkts_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                  u_long *ipSystemStatsOutBcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsOutBcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsOutBcastPkts_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsOutBcastPkts_val_ptr) =
        rowreq_ctx->data.ipSystemStatsOutBcastPkts;
    return MFD_SUCCESS;
}

int ipSystemStatsHCOutBcastPkts_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    unsigned long long *ipSystemStatsHCOutBcastPkts_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsHCOutBcastPkts_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsHCOutBcastPkts_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsHCOutBcastPkts_val_ptr) =
        rowreq_ctx->data.ipSystemStatsHCOutBcastPkts;
    return MFD_SUCCESS;
}

int ipSystemStatsDiscontinuityTime_get(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
    long *ipSystemStatsDiscontinuityTime_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsDiscontinuityTime_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsDiscontinuityTime_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsDiscontinuityTime_val_ptr) =
        rowreq_ctx->data.ipSystemStatsDiscontinuityTime;
    return MFD_SUCCESS;
}

int ipSystemStatsRefreshRate_get(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 u_long *ipSystemStatsRefreshRate_val_ptr) {
    netsnmp_assert(NULL != ipSystemStatsRefreshRate_val_ptr);
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsRefreshRate_get",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);

    (*ipSystemStatsRefreshRate_val_ptr) =
        rowreq_ctx->data.ipSystemStatsRefreshRate;
    return MFD_SUCCESS;
}
