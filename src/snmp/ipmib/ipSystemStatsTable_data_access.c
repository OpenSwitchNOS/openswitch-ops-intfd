#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "IP_MIB_custom.h"
#include "ipSystemStatsTable.h"
#include "ipSystemStatsTable_data_access.h"

#include "openswitch-idl.h"
#include "ovsdb-idl.h"
#include "vswitch-idl.h"
#include "openvswitch/vlog.h"

int ipSystemStatsTable_init_data(
    ipSystemStatsTable_registration *ipSystemStatsTable_reg) {
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsTable_init_data",
                "called\n"));
    return MFD_SUCCESS;
}

void ipSystemStatsTable_container_init(netsnmp_container **container_ptr_ptr,
                                       netsnmp_cache *cache) {
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsTable_container_init",
                "called\n"));
    if (NULL == container_ptr_ptr) {
        snmp_log(LOG_ERR,
                 "bad container param to ipSystemStatsTable_container_init\n");
        return;
    }
    *container_ptr_ptr = NULL;
    if (NULL == cache) {
        snmp_log(LOG_ERR,
                 "bad cache param to ipSystemStatsTable_container_init\n");
        return;
    }
    cache->timeout = IPSYSTEMSTATSTABLE_CACHE_TIMEOUT;
}

void ipSystemStatsTable_container_shutdown(netsnmp_container *container_ptr) {
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsTable_container_shutdown",
         "called\n"));
    if (NULL == container_ptr) {
        snmp_log(LOG_ERR,
                 "bad params to ipSystemStatsTable_container_shutdown\n");
        return;
    }
}

int ipSystemStatsTable_container_load(netsnmp_container *container) {
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsTable_container_load",
                "called\n"));
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx;
    size_t count = 0;

    const struct ovsrec_port *port_row = NULL;

    long ipSystemStatsIPVersion;

    u_long ipSystemStatsInReceivesv4 = 0;
    unsigned long long ipSystemStatsHCInReceivesv4 = 0;
    u_long ipSystemStatsInOctetsv4 = 0;
    unsigned long long ipSystemStatsHCInOctetsv4 = 0;
    u_long ipSystemStatsInHdrErrorsv4 = 0;
    u_long ipSystemStatsInNoRoutesv4 = 0;
    u_long ipSystemStatsInAddrErrorsv4 = 0;
    u_long ipSystemStatsInUnknownProtosv4 = 0;
    u_long ipSystemStatsInTruncatedPktsv4 = 0;
    u_long ipSystemStatsInForwDatagramsv4 = 0;
    unsigned long long ipSystemStatsHCInForwDatagramsv4 = 0;
    u_long ipSystemStatsReasmReqdsv4 = 0;
    u_long ipSystemStatsReasmOKsv4 = 0;
    u_long ipSystemStatsReasmFailsv4 = 0;
    u_long ipSystemStatsInDiscardsv4 = 0;
    u_long ipSystemStatsInDeliversv4 = 0;
    unsigned long long ipSystemStatsHCInDeliversv4 = 0;
    u_long ipSystemStatsOutRequestsv4 = 0;
    unsigned long long ipSystemStatsHCOutRequestsv4 = 0;
    u_long ipSystemStatsOutNoRoutesv4 = 0;
    u_long ipSystemStatsOutForwDatagramsv4 = 0;
    unsigned long long ipSystemStatsHCOutForwDatagramsv4 = 0;
    u_long ipSystemStatsOutDiscardsv4 = 0;
    u_long ipSystemStatsOutFragReqdsv4 = 0;
    u_long ipSystemStatsOutFragOKsv4 = 0;
    u_long ipSystemStatsOutFragFailsv4 = 0;
    u_long ipSystemStatsOutFragCreatesv4 = 0;
    u_long ipSystemStatsOutTransmitsv4 = 0;
    unsigned long long ipSystemStatsHCOutTransmitsv4 = 0;
    u_long ipSystemStatsOutOctetsv4 = 0;
    unsigned long long ipSystemStatsHCOutOctetsv4 = 0;
    u_long ipSystemStatsInMcastPktsv4 = 0;
    unsigned long long ipSystemStatsHCInMcastPktsv4 = 0;
    u_long ipSystemStatsInMcastOctetsv4 = 0;
    unsigned long long ipSystemStatsHCInMcastOctetsv4 = 0;
    u_long ipSystemStatsOutMcastPktsv4 = 0;
    unsigned long long ipSystemStatsHCOutMcastPktsv4 = 0;
    u_long ipSystemStatsOutMcastOctetsv4 = 0;
    unsigned long long ipSystemStatsHCOutMcastOctetsv4 = 0;
    u_long ipSystemStatsInBcastPktsv4 = 0;
    unsigned long long ipSystemStatsHCInBcastPktsv4 = 0;
    u_long ipSystemStatsOutBcastPktsv4 = 0;
    unsigned long long ipSystemStatsHCOutBcastPktsv4 = 0;
    long ipSystemStatsDiscontinuityTimev4 = 0;
    u_long ipSystemStatsRefreshRatev4 = 0;

    u_long ipSystemStatsInReceivesv6 = 0;
    unsigned long long ipSystemStatsHCInReceivesv6 = 0;
    u_long ipSystemStatsInOctetsv6 = 0;
    unsigned long long ipSystemStatsHCInOctetsv6 = 0;
    u_long ipSystemStatsInHdrErrorsv6 = 0;
    u_long ipSystemStatsInNoRoutesv6 = 0;
    u_long ipSystemStatsInAddrErrorsv6 = 0;
    u_long ipSystemStatsInUnknownProtosv6 = 0;
    u_long ipSystemStatsInTruncatedPktsv6 = 0;
    u_long ipSystemStatsInForwDatagramsv6 = 0;
    unsigned long long ipSystemStatsHCInForwDatagramsv6 = 0;
    u_long ipSystemStatsReasmReqdsv6 = 0;
    u_long ipSystemStatsReasmOKsv6 = 0;
    u_long ipSystemStatsReasmFailsv6 = 0;
    u_long ipSystemStatsInDiscardsv6 = 0;
    u_long ipSystemStatsInDeliversv6 = 0;
    unsigned long long ipSystemStatsHCInDeliversv6 = 0;
    u_long ipSystemStatsOutRequestsv6 = 0;
    unsigned long long ipSystemStatsHCOutRequestsv6 = 0;
    u_long ipSystemStatsOutNoRoutesv6 = 0;
    u_long ipSystemStatsOutForwDatagramsv6 = 0;
    unsigned long long ipSystemStatsHCOutForwDatagramsv6 = 0;
    u_long ipSystemStatsOutDiscardsv6 = 0;
    u_long ipSystemStatsOutFragReqdsv6 = 0;
    u_long ipSystemStatsOutFragOKsv6 = 0;
    u_long ipSystemStatsOutFragFailsv6 = 0;
    u_long ipSystemStatsOutFragCreatesv6 = 0;
    u_long ipSystemStatsOutTransmitsv6 = 0;
    unsigned long long ipSystemStatsHCOutTransmitsv6 = 0;
    u_long ipSystemStatsOutOctetsv6 = 0;
    unsigned long long ipSystemStatsHCOutOctetsv6 = 0;
    u_long ipSystemStatsInMcastPktsv6 = 0;
    unsigned long long ipSystemStatsHCInMcastPktsv6 = 0;
    u_long ipSystemStatsInMcastOctetsv6 = 0;
    unsigned long long ipSystemStatsHCInMcastOctetsv6 = 0;
    u_long ipSystemStatsOutMcastPktsv6 = 0;
    unsigned long long ipSystemStatsHCOutMcastPktsv6 = 0;
    u_long ipSystemStatsOutMcastOctetsv6 = 0;
    unsigned long long ipSystemStatsHCOutMcastOctetsv6 = 0;
    u_long ipSystemStatsInBcastPktsv6 = 0;
    unsigned long long ipSystemStatsHCInBcastPktsv6 = 0;
    u_long ipSystemStatsOutBcastPktsv6 = 0;
    unsigned long long ipSystemStatsHCOutBcastPktsv6 = 0;
    long ipSystemStatsDiscontinuityTimev6 = 0;
    u_long ipSystemStatsRefreshRatev6 = 0;

    port_row = ovsrec_port_first(idl);
    if (!port_row) {
        snmp_log(LOG_ERR, "not able to fetch port row");
        return -1;
    }

    OVSREC_PORT_FOR_EACH(port_row, idl) {
        const struct ovsrec_interface *interface_row = NULL;

        if(portTable_inetv4(idl, port_row, NULL)){
            interface_row = *port_row->interfaces;

            ipSystemStatsInReceivesv4 += ipIfStatsInReceives_custom_function_v4(interface_row);
            ipSystemStatsHCInReceivesv4 += ipIfStatsInReceives_custom_function_v4(interface_row);

            ipSystemStatsInOctetsv4 += ipIfStatsInOctets_custom_function_v4(interface_row);
            ipSystemStatsHCInOctetsv4 += ipIfStatsInOctets_custom_function_v4(interface_row);

            ipSystemStatsOutTransmitsv4 += ipIfStatsOutTransmits_custom_function_v4(interface_row);
            ipSystemStatsHCOutTransmitsv4 += ipIfStatsOutTransmits_custom_function_v4(interface_row);

            ipSystemStatsOutOctetsv4 += ipIfStatsOutOctets_custom_function_v4(interface_row);
            ipSystemStatsHCOutOctetsv4 += ipIfStatsOutOctets_custom_function_v4(interface_row);

            ipSystemStatsInMcastPktsv4 += ipIfStatsInMcastPkts_custom_function_v4(interface_row);
            ipSystemStatsHCInMcastPktsv4 += ipIfStatsInMcastPkts_custom_function_v4(interface_row);

            ipSystemStatsInMcastOctetsv4 += ipIfStatsInMcastOctets_custom_function_v4(interface_row);
            ipSystemStatsHCInMcastOctetsv4 += ipIfStatsInMcastOctets_custom_function_v4(interface_row);

            ipSystemStatsOutMcastPktsv4 += ipIfStatsOutMcastPkts_custom_function_v4(interface_row);
            ipSystemStatsHCOutMcastPktsv4 += ipIfStatsOutMcastPkts_custom_function_v4(interface_row);

            ipSystemStatsOutMcastOctetsv4 += ipIfStatsOutMcastOctets_custom_function_v4(interface_row);
            ipSystemStatsHCOutMcastOctetsv4 += ipIfStatsOutMcastOctets_custom_function_v4(interface_row);
        }
        else if (portTable_inetv6(idl, port_row, NULL)){
            interface_row = *port_row->interfaces;

            ipSystemStatsInReceivesv6 += ipIfStatsInReceives_custom_function_v6(interface_row);
            ipSystemStatsHCInReceivesv6 += ipIfStatsInReceives_custom_function_v6(interface_row);

            ipSystemStatsInOctetsv6 += ipIfStatsInOctets_custom_function_v6(interface_row);
            ipSystemStatsHCInOctetsv6 += ipIfStatsInOctets_custom_function_v6(interface_row);

            ipSystemStatsOutTransmitsv6 += ipIfStatsOutTransmits_custom_function_v6(interface_row);
            ipSystemStatsHCOutTransmitsv6 += ipIfStatsOutTransmits_custom_function_v6(interface_row);

            ipSystemStatsOutOctetsv6 += ipIfStatsOutOctets_custom_function_v6(interface_row);
            ipSystemStatsHCOutOctetsv6 += ipIfStatsOutOctets_custom_function_v6(interface_row);

            ipSystemStatsInMcastPktsv6 += ipIfStatsInMcastPkts_custom_function_v6(interface_row);
            ipSystemStatsHCInMcastPktsv6 += ipIfStatsInMcastPkts_custom_function_v6(interface_row);

            ipSystemStatsInMcastOctetsv6 += ipIfStatsInMcastOctets_custom_function_v6(interface_row);
            ipSystemStatsHCInMcastOctetsv6 += ipIfStatsInMcastOctets_custom_function_v6(interface_row);

            ipSystemStatsOutMcastPktsv6 += ipIfStatsOutMcastPkts_custom_function_v6(interface_row);
            ipSystemStatsHCOutMcastPktsv6 += ipIfStatsOutMcastPkts_custom_function_v6(interface_row);

            ipSystemStatsOutMcastOctetsv6 += ipIfStatsOutMcastOctets_custom_function_v6(interface_row);
            ipSystemStatsHCOutMcastOctetsv6 += ipIfStatsOutMcastOctets_custom_function_v6(interface_row);
        }
    }

    ipSystemStatsIPVersion = 1;
    rowreq_ctx = ipSystemStatsTable_allocate_rowreq_ctx(NULL);
    if (rowreq_ctx == NULL) {
        snmp_log(LOG_ERR, "memory allocation failed");
        return MFD_RESOURCE_UNAVAILABLE;
    }
    if (MFD_SUCCESS != ipSystemStatsTable_indexes_set(
                            rowreq_ctx, ipSystemStatsIPVersion)) {
        snmp_log(LOG_ERR, "error setting indexes while loading");
        ipSystemStatsTable_release_rowreq_ctx(rowreq_ctx);
        return MFD_RESOURCE_UNAVAILABLE;
    }

    rowreq_ctx->data.ipSystemStatsInReceives = ipSystemStatsInReceivesv4;
    rowreq_ctx->data.ipSystemStatsHCInReceives = ipSystemStatsHCInReceivesv4;
    rowreq_ctx->data.ipSystemStatsInOctets = ipSystemStatsInOctetsv4;
    rowreq_ctx->data.ipSystemStatsHCInOctets = ipSystemStatsHCInOctetsv4;
    rowreq_ctx->data.ipSystemStatsInHdrErrors = ipSystemStatsInHdrErrorsv4;
    rowreq_ctx->data.ipSystemStatsInNoRoutes = ipSystemStatsInNoRoutesv4;
    rowreq_ctx->data.ipSystemStatsInAddrErrors = ipSystemStatsInAddrErrorsv4;
    rowreq_ctx->data.ipSystemStatsInUnknownProtos =
        ipSystemStatsInUnknownProtosv4;
    rowreq_ctx->data.ipSystemStatsInTruncatedPkts =
        ipSystemStatsInTruncatedPktsv4;
    rowreq_ctx->data.ipSystemStatsInForwDatagrams =
        ipSystemStatsInForwDatagramsv4;
    rowreq_ctx->data.ipSystemStatsHCInForwDatagrams =
        ipSystemStatsHCInForwDatagramsv4;
    rowreq_ctx->data.ipSystemStatsReasmReqds = ipSystemStatsReasmReqdsv4;
    rowreq_ctx->data.ipSystemStatsReasmOKs = ipSystemStatsReasmOKsv4;
    rowreq_ctx->data.ipSystemStatsReasmFails = ipSystemStatsReasmFailsv4;
    rowreq_ctx->data.ipSystemStatsInDiscards = ipSystemStatsInDiscardsv4;
    rowreq_ctx->data.ipSystemStatsInDelivers = ipSystemStatsInDeliversv4;
    rowreq_ctx->data.ipSystemStatsHCInDelivers = ipSystemStatsHCInDeliversv4;
    rowreq_ctx->data.ipSystemStatsOutRequests = ipSystemStatsOutRequestsv4;
    rowreq_ctx->data.ipSystemStatsHCOutRequests =
        ipSystemStatsHCOutRequestsv4;
    rowreq_ctx->data.ipSystemStatsOutNoRoutes = ipSystemStatsOutNoRoutesv4;
    rowreq_ctx->data.ipSystemStatsOutForwDatagrams =
        ipSystemStatsOutForwDatagramsv4;
    rowreq_ctx->data.ipSystemStatsHCOutForwDatagrams =
        ipSystemStatsHCOutForwDatagramsv4;
    rowreq_ctx->data.ipSystemStatsOutDiscards = ipSystemStatsOutDiscardsv4;
    rowreq_ctx->data.ipSystemStatsOutFragReqds = ipSystemStatsOutFragReqdsv4;
    rowreq_ctx->data.ipSystemStatsOutFragOKs = ipSystemStatsOutFragOKsv4;
    rowreq_ctx->data.ipSystemStatsOutFragFails = ipSystemStatsOutFragFailsv4;
    rowreq_ctx->data.ipSystemStatsOutFragCreates =
        ipSystemStatsOutFragCreatesv4;
    rowreq_ctx->data.ipSystemStatsOutTransmits = ipSystemStatsOutTransmitsv4;
    rowreq_ctx->data.ipSystemStatsHCOutTransmits =
        ipSystemStatsHCOutTransmitsv4;
    rowreq_ctx->data.ipSystemStatsOutOctets = ipSystemStatsOutOctetsv4;
    rowreq_ctx->data.ipSystemStatsHCOutOctets = ipSystemStatsHCOutOctetsv4;
    rowreq_ctx->data.ipSystemStatsInMcastPkts = ipSystemStatsInMcastPktsv4;
    rowreq_ctx->data.ipSystemStatsHCInMcastPkts =
        ipSystemStatsHCInMcastPktsv4;
    rowreq_ctx->data.ipSystemStatsInMcastOctets =
        ipSystemStatsInMcastOctetsv4;
    rowreq_ctx->data.ipSystemStatsHCInMcastOctets =
        ipSystemStatsHCInMcastOctetsv4;
    rowreq_ctx->data.ipSystemStatsOutMcastPkts = ipSystemStatsOutMcastPktsv4;
    rowreq_ctx->data.ipSystemStatsHCOutMcastPkts =
        ipSystemStatsHCOutMcastPktsv4;
    rowreq_ctx->data.ipSystemStatsOutMcastOctets =
        ipSystemStatsOutMcastOctetsv4;
    rowreq_ctx->data.ipSystemStatsHCOutMcastOctets =
        ipSystemStatsHCOutMcastOctetsv4;
    rowreq_ctx->data.ipSystemStatsInBcastPkts = ipSystemStatsInBcastPktsv4;
    rowreq_ctx->data.ipSystemStatsHCInBcastPkts =
        ipSystemStatsHCInBcastPktsv4;
    rowreq_ctx->data.ipSystemStatsOutBcastPkts = ipSystemStatsOutBcastPktsv4;
    rowreq_ctx->data.ipSystemStatsHCOutBcastPkts =
        ipSystemStatsHCOutBcastPktsv4;
    rowreq_ctx->data.ipSystemStatsDiscontinuityTime =
        ipSystemStatsDiscontinuityTimev4;
    rowreq_ctx->data.ipSystemStatsRefreshRate = ipSystemStatsRefreshRatev4;
    CONTAINER_INSERT(container, rowreq_ctx);
    ++count;

    ipSystemStatsIPVersion = 2;
    rowreq_ctx = ipSystemStatsTable_allocate_rowreq_ctx(NULL);
    if (rowreq_ctx == NULL) {
        snmp_log(LOG_ERR, "memory allocation failed");
        return MFD_RESOURCE_UNAVAILABLE;
    }
    if (MFD_SUCCESS != ipSystemStatsTable_indexes_set(
                            rowreq_ctx, ipSystemStatsIPVersion)) {
        snmp_log(LOG_ERR, "error setting indexes while loading");
        ipSystemStatsTable_release_rowreq_ctx(rowreq_ctx);
        return MFD_RESOURCE_UNAVAILABLE;
    }

    rowreq_ctx->data.ipSystemStatsInReceives = ipSystemStatsInReceivesv6;
    rowreq_ctx->data.ipSystemStatsHCInReceives = ipSystemStatsHCInReceivesv6;
    rowreq_ctx->data.ipSystemStatsInOctets = ipSystemStatsInOctetsv6;
    rowreq_ctx->data.ipSystemStatsHCInOctets = ipSystemStatsHCInOctetsv6;
    rowreq_ctx->data.ipSystemStatsInHdrErrors = ipSystemStatsInHdrErrorsv6;
    rowreq_ctx->data.ipSystemStatsInNoRoutes = ipSystemStatsInNoRoutesv6;
    rowreq_ctx->data.ipSystemStatsInAddrErrors = ipSystemStatsInAddrErrorsv6;
    rowreq_ctx->data.ipSystemStatsInUnknownProtos =
        ipSystemStatsInUnknownProtosv6;
    rowreq_ctx->data.ipSystemStatsInTruncatedPkts =
        ipSystemStatsInTruncatedPktsv6;
    rowreq_ctx->data.ipSystemStatsInForwDatagrams =
        ipSystemStatsInForwDatagramsv6;
    rowreq_ctx->data.ipSystemStatsHCInForwDatagrams =
        ipSystemStatsHCInForwDatagramsv6;
    rowreq_ctx->data.ipSystemStatsReasmReqds = ipSystemStatsReasmReqdsv6;
    rowreq_ctx->data.ipSystemStatsReasmOKs = ipSystemStatsReasmOKsv6;
    rowreq_ctx->data.ipSystemStatsReasmFails = ipSystemStatsReasmFailsv6;
    rowreq_ctx->data.ipSystemStatsInDiscards = ipSystemStatsInDiscardsv6;
    rowreq_ctx->data.ipSystemStatsInDelivers = ipSystemStatsInDeliversv6;
    rowreq_ctx->data.ipSystemStatsHCInDelivers = ipSystemStatsHCInDeliversv6;
    rowreq_ctx->data.ipSystemStatsOutRequests = ipSystemStatsOutRequestsv6;
    rowreq_ctx->data.ipSystemStatsHCOutRequests =
        ipSystemStatsHCOutRequestsv6;
    rowreq_ctx->data.ipSystemStatsOutNoRoutes = ipSystemStatsOutNoRoutesv6;
    rowreq_ctx->data.ipSystemStatsOutForwDatagrams =
        ipSystemStatsOutForwDatagramsv6;
    rowreq_ctx->data.ipSystemStatsHCOutForwDatagrams =
        ipSystemStatsHCOutForwDatagramsv6;
    rowreq_ctx->data.ipSystemStatsOutDiscards = ipSystemStatsOutDiscardsv6;
    rowreq_ctx->data.ipSystemStatsOutFragReqds = ipSystemStatsOutFragReqdsv6;
    rowreq_ctx->data.ipSystemStatsOutFragOKs = ipSystemStatsOutFragOKsv6;
    rowreq_ctx->data.ipSystemStatsOutFragFails = ipSystemStatsOutFragFailsv6;
    rowreq_ctx->data.ipSystemStatsOutFragCreates =
        ipSystemStatsOutFragCreatesv6;
    rowreq_ctx->data.ipSystemStatsOutTransmits = ipSystemStatsOutTransmitsv6;
    rowreq_ctx->data.ipSystemStatsHCOutTransmits =
        ipSystemStatsHCOutTransmitsv6;
    rowreq_ctx->data.ipSystemStatsOutOctets = ipSystemStatsOutOctetsv6;
    rowreq_ctx->data.ipSystemStatsHCOutOctets = ipSystemStatsHCOutOctetsv6;
    rowreq_ctx->data.ipSystemStatsInMcastPkts = ipSystemStatsInMcastPktsv6;
    rowreq_ctx->data.ipSystemStatsHCInMcastPkts =
        ipSystemStatsHCInMcastPktsv6;
    rowreq_ctx->data.ipSystemStatsInMcastOctets =
        ipSystemStatsInMcastOctetsv6;
    rowreq_ctx->data.ipSystemStatsHCInMcastOctets =
        ipSystemStatsHCInMcastOctetsv6;
    rowreq_ctx->data.ipSystemStatsOutMcastPkts = ipSystemStatsOutMcastPktsv6;
    rowreq_ctx->data.ipSystemStatsHCOutMcastPkts =
        ipSystemStatsHCOutMcastPktsv6;
    rowreq_ctx->data.ipSystemStatsOutMcastOctets =
        ipSystemStatsOutMcastOctetsv6;
    rowreq_ctx->data.ipSystemStatsHCOutMcastOctets =
        ipSystemStatsHCOutMcastOctetsv6;
    rowreq_ctx->data.ipSystemStatsInBcastPkts = ipSystemStatsInBcastPktsv6;
    rowreq_ctx->data.ipSystemStatsHCInBcastPkts =
        ipSystemStatsHCInBcastPktsv6;
    rowreq_ctx->data.ipSystemStatsOutBcastPkts = ipSystemStatsOutBcastPktsv6;
    rowreq_ctx->data.ipSystemStatsHCOutBcastPkts =
        ipSystemStatsHCOutBcastPktsv6;
    rowreq_ctx->data.ipSystemStatsDiscontinuityTime =
        ipSystemStatsDiscontinuityTimev6;
    rowreq_ctx->data.ipSystemStatsRefreshRate = ipSystemStatsRefreshRatev6;
    CONTAINER_INSERT(container, rowreq_ctx);
    ++count;

    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsTable_container_load",
                "inserted %d records\n", (int)count));
    return MFD_SUCCESS;
}

void ipSystemStatsTable_container_free(netsnmp_container *container) {
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsTable_container_free",
                "called\n"));
}

int ipSystemStatsTable_row_prep(ipSystemStatsTable_rowreq_ctx *rowreq_ctx) {
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsTable_row_prep", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);
    return MFD_SUCCESS;
}
