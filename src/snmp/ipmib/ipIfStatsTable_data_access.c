#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "IP_MIB_custom.h"
#include "ipIfStatsTable.h"
#include "ipIfStatsTable_data_access.h"

#include "openswitch-idl.h"
#include "ovsdb-idl.h"
#include "vswitch-idl.h"
#include "openvswitch/vlog.h"

int ipIfStatsTable_init_data(ipIfStatsTable_registration *ipIfStatsTable_reg) {
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsTable_init_data", "called\n"));
    return MFD_SUCCESS;
}

void ipIfStatsTable_container_init(netsnmp_container **container_ptr_ptr,
                                   netsnmp_cache *cache) {
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsTable_container_init", "called\n"));
    if (NULL == container_ptr_ptr) {
        snmp_log(LOG_ERR,
                 "bad container param to ipIfStatsTable_container_init\n");
        return;
    }
    *container_ptr_ptr = NULL;
    if (NULL == cache) {
        snmp_log(LOG_ERR, "bad cache param to ipIfStatsTable_container_init\n");
        return;
    }
    cache->timeout = IPIFSTATSTABLE_CACHE_TIMEOUT;
}

void ipIfStatsTable_container_shutdown(netsnmp_container *container_ptr) {
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsTable_container_shutdown",
                "called\n"));
    if (NULL == container_ptr) {
        snmp_log(LOG_ERR, "bad params to ipIfStatsTable_container_shutdown\n");
        return;
    }
}

int ipIfStatsTable_container_load(netsnmp_container *container) {
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsTable_container_load", "called\n"));
    ipIfStatsTable_rowreq_ctx *rowreq_ctx;
    size_t count = 0;

    const struct ovsrec_port *port_row = NULL;

    long ipIfStatsIPVersion;
    long ipIfStatsIfIndex;

    u_long ipIfStatsInReceives = 0;
    unsigned long long ipIfStatsHCInReceives = 0;
    u_long ipIfStatsInOctets = 0;
    unsigned long long ipIfStatsHCInOctets = 0;
    u_long ipIfStatsInHdrErrors = 0;
    u_long ipIfStatsInNoRoutes = 0;
    u_long ipIfStatsInAddrErrors = 0;
    u_long ipIfStatsInUnknownProtos = 0;
    u_long ipIfStatsInTruncatedPkts = 0;
    u_long ipIfStatsInForwDatagrams = 0;
    unsigned long long ipIfStatsHCInForwDatagrams = 0;
    u_long ipIfStatsReasmReqds = 0;
    u_long ipIfStatsReasmOKs = 0;
    u_long ipIfStatsReasmFails = 0;
    u_long ipIfStatsInDiscards = 0;
    u_long ipIfStatsInDelivers = 0;
    unsigned long long ipIfStatsHCInDelivers = 0;
    u_long ipIfStatsOutRequests = 0;
    unsigned long long ipIfStatsHCOutRequests = 0;
    u_long ipIfStatsOutForwDatagrams = 0;
    unsigned long long ipIfStatsHCOutForwDatagrams = 0;
    u_long ipIfStatsOutDiscards = 0;
    u_long ipIfStatsOutFragReqds = 0;
    u_long ipIfStatsOutFragOKs = 0;
    u_long ipIfStatsOutFragFails = 0;
    u_long ipIfStatsOutFragCreates = 0;
    u_long ipIfStatsOutTransmits = 0;
    unsigned long long ipIfStatsHCOutTransmits = 0;
    u_long ipIfStatsOutOctets = 0;
    unsigned long long ipIfStatsHCOutOctets = 0;
    u_long ipIfStatsInMcastPkts = 0;
    unsigned long long ipIfStatsHCInMcastPkts = 0;
    u_long ipIfStatsInMcastOctets = 0;
    unsigned long long ipIfStatsHCInMcastOctets = 0;
    u_long ipIfStatsOutMcastPkts = 0;
    unsigned long long ipIfStatsHCOutMcastPkts = 0;
    u_long ipIfStatsOutMcastOctets = 0;
    unsigned long long ipIfStatsHCOutMcastOctets = 0;
    u_long ipIfStatsInBcastPkts = 0;
    unsigned long long ipIfStatsHCInBcastPkts = 0;
    u_long ipIfStatsOutBcastPkts = 0;
    unsigned long long ipIfStatsHCOutBcastPkts = 0;
    long ipIfStatsDiscontinuityTime = 0;
    u_long ipIfStatsRefreshRate = 0;

    port_row = ovsrec_port_first(idl);
    if (!port_row) {
        snmp_log(LOG_ERR, "not able to fetch port row");
        return -1;
    }

    OVSREC_PORT_FOR_EACH(port_row, idl) {
        if(port_row->interfaces == NULL || portTable_skip_function(idl, port_row)) {
            continue;
        }

        if(portTable_inetv4(idl, port_row, &ipIfStatsIfIndex)){
            ipIfStatsIPVersion = 1;

            const struct ovsrec_interface *interface_row = *port_row->interfaces;

            ipIfStatsInReceives = ipIfStatsInReceives_custom_function_v4(interface_row);
            ipIfStatsHCInReceives = ipIfStatsInReceives_custom_function_v4(interface_row);

            ipIfStatsInOctets = ipIfStatsInOctets_custom_function_v4(interface_row);
            ipIfStatsHCInOctets = ipIfStatsInOctets_custom_function_v4(interface_row);

            ipIfStatsOutTransmits = ipIfStatsOutTransmits_custom_function_v4(interface_row);
            ipIfStatsHCOutTransmits = ipIfStatsOutTransmits_custom_function_v4(interface_row);

            ipIfStatsOutOctets = ipIfStatsOutOctets_custom_function_v4(interface_row);
            ipIfStatsHCOutOctets = ipIfStatsOutOctets_custom_function_v4(interface_row);

            ipIfStatsInMcastPkts = ipIfStatsInMcastPkts_custom_function_v4(interface_row);
            ipIfStatsHCInMcastPkts = ipIfStatsInMcastPkts_custom_function_v4(interface_row);

            ipIfStatsInMcastOctets = ipIfStatsInMcastOctets_custom_function_v4(interface_row);
            ipIfStatsHCInMcastOctets = ipIfStatsInMcastOctets_custom_function_v4(interface_row);

            ipIfStatsOutMcastPkts = ipIfStatsOutMcastPkts_custom_function_v4(interface_row);
            ipIfStatsHCOutMcastPkts = ipIfStatsOutMcastPkts_custom_function_v4(interface_row);

            ipIfStatsOutMcastOctets = ipIfStatsOutMcastOctets_custom_function_v4(interface_row);
            ipIfStatsHCOutMcastOctets = ipIfStatsOutMcastOctets_custom_function_v4(interface_row);

            rowreq_ctx = ipIfStatsTable_allocate_rowreq_ctx(NULL);
            if (rowreq_ctx == NULL) {
                snmp_log(LOG_ERR, "memory allocation failed");
                return MFD_RESOURCE_UNAVAILABLE;
            }
            if (MFD_SUCCESS != ipIfStatsTable_indexes_set(rowreq_ctx,
                                                        ipIfStatsIPVersion,
                                                        ipIfStatsIfIndex)) {
                snmp_log(LOG_ERR, "error setting indexes while loading");
                ipIfStatsTable_release_rowreq_ctx(rowreq_ctx);
                continue;
            }

            rowreq_ctx->data.ipIfStatsInReceives = ipIfStatsInReceives;
            rowreq_ctx->data.ipIfStatsHCInReceives = ipIfStatsHCInReceives;
            rowreq_ctx->data.ipIfStatsInOctets = ipIfStatsInOctets;
            rowreq_ctx->data.ipIfStatsHCInOctets = ipIfStatsHCInOctets;
            rowreq_ctx->data.ipIfStatsInHdrErrors = ipIfStatsInHdrErrors;
            rowreq_ctx->data.ipIfStatsInNoRoutes = ipIfStatsInNoRoutes;
            rowreq_ctx->data.ipIfStatsInAddrErrors = ipIfStatsInAddrErrors;
            rowreq_ctx->data.ipIfStatsInUnknownProtos = ipIfStatsInUnknownProtos;
            rowreq_ctx->data.ipIfStatsInTruncatedPkts = ipIfStatsInTruncatedPkts;
            rowreq_ctx->data.ipIfStatsInForwDatagrams = ipIfStatsInForwDatagrams;
            rowreq_ctx->data.ipIfStatsHCInForwDatagrams =
                ipIfStatsHCInForwDatagrams;
            rowreq_ctx->data.ipIfStatsReasmReqds = ipIfStatsReasmReqds;
            rowreq_ctx->data.ipIfStatsReasmOKs = ipIfStatsReasmOKs;
            rowreq_ctx->data.ipIfStatsReasmFails = ipIfStatsReasmFails;
            rowreq_ctx->data.ipIfStatsInDiscards = ipIfStatsInDiscards;
            rowreq_ctx->data.ipIfStatsInDelivers = ipIfStatsInDelivers;
            rowreq_ctx->data.ipIfStatsHCInDelivers = ipIfStatsHCInDelivers;
            rowreq_ctx->data.ipIfStatsOutRequests = ipIfStatsOutRequests;
            rowreq_ctx->data.ipIfStatsHCOutRequests = ipIfStatsHCOutRequests;
            rowreq_ctx->data.ipIfStatsOutForwDatagrams = ipIfStatsOutForwDatagrams;
            rowreq_ctx->data.ipIfStatsHCOutForwDatagrams =
                ipIfStatsHCOutForwDatagrams;
            rowreq_ctx->data.ipIfStatsOutDiscards = ipIfStatsOutDiscards;
            rowreq_ctx->data.ipIfStatsOutFragReqds = ipIfStatsOutFragReqds;
            rowreq_ctx->data.ipIfStatsOutFragOKs = ipIfStatsOutFragOKs;
            rowreq_ctx->data.ipIfStatsOutFragFails = ipIfStatsOutFragFails;
            rowreq_ctx->data.ipIfStatsOutFragCreates = ipIfStatsOutFragCreates;
            rowreq_ctx->data.ipIfStatsOutTransmits = ipIfStatsOutTransmits;
            rowreq_ctx->data.ipIfStatsHCOutTransmits = ipIfStatsHCOutTransmits;
            rowreq_ctx->data.ipIfStatsOutOctets = ipIfStatsOutOctets;
            rowreq_ctx->data.ipIfStatsHCOutOctets = ipIfStatsHCOutOctets;
            rowreq_ctx->data.ipIfStatsInMcastPkts = ipIfStatsInMcastPkts;
            rowreq_ctx->data.ipIfStatsHCInMcastPkts = ipIfStatsHCInMcastPkts;
            rowreq_ctx->data.ipIfStatsInMcastOctets = ipIfStatsInMcastOctets;
            rowreq_ctx->data.ipIfStatsHCInMcastOctets = ipIfStatsHCInMcastOctets;
            rowreq_ctx->data.ipIfStatsOutMcastPkts = ipIfStatsOutMcastPkts;
            rowreq_ctx->data.ipIfStatsHCOutMcastPkts = ipIfStatsHCOutMcastPkts;
            rowreq_ctx->data.ipIfStatsOutMcastOctets = ipIfStatsOutMcastOctets;
            rowreq_ctx->data.ipIfStatsHCOutMcastOctets = ipIfStatsHCOutMcastOctets;
            rowreq_ctx->data.ipIfStatsInBcastPkts = ipIfStatsInBcastPkts;
            rowreq_ctx->data.ipIfStatsHCInBcastPkts = ipIfStatsHCInBcastPkts;
            rowreq_ctx->data.ipIfStatsOutBcastPkts = ipIfStatsOutBcastPkts;
            rowreq_ctx->data.ipIfStatsHCOutBcastPkts = ipIfStatsHCOutBcastPkts;
            rowreq_ctx->data.ipIfStatsDiscontinuityTime =
                ipIfStatsDiscontinuityTime;
            rowreq_ctx->data.ipIfStatsRefreshRate = ipIfStatsRefreshRate;
            CONTAINER_INSERT(container, rowreq_ctx);
            ++count;
        }
        if(portTable_inetv6(idl, port_row, &ipIfStatsIfIndex)){
            ipIfStatsIPVersion = 2;

            const struct ovsrec_interface *interface_row = *port_row->interfaces;

            ipIfStatsInReceives = ipIfStatsInReceives_custom_function_v6(interface_row);
            ipIfStatsHCInReceives = ipIfStatsInReceives_custom_function_v6(interface_row);

            ipIfStatsInOctets = ipIfStatsInOctets_custom_function_v6(interface_row);
            ipIfStatsHCInOctets = ipIfStatsInOctets_custom_function_v6(interface_row);

            ipIfStatsOutTransmits = ipIfStatsOutTransmits_custom_function_v6(interface_row);
            ipIfStatsHCOutTransmits = ipIfStatsOutTransmits_custom_function_v6(interface_row);

            ipIfStatsOutOctets = ipIfStatsOutOctets_custom_function_v6(interface_row);
            ipIfStatsHCOutOctets = ipIfStatsOutOctets_custom_function_v6(interface_row);

            ipIfStatsInMcastPkts = ipIfStatsInMcastPkts_custom_function_v6(interface_row);
            ipIfStatsHCInMcastPkts = ipIfStatsInMcastPkts_custom_function_v6(interface_row);

            ipIfStatsInMcastOctets = ipIfStatsInMcastOctets_custom_function_v6(interface_row);
            ipIfStatsHCInMcastOctets = ipIfStatsInMcastOctets_custom_function_v6(interface_row);

            ipIfStatsOutMcastPkts = ipIfStatsOutMcastPkts_custom_function_v6(interface_row);
            ipIfStatsHCOutMcastPkts = ipIfStatsOutMcastPkts_custom_function_v6(interface_row);

            ipIfStatsOutMcastOctets = ipIfStatsOutMcastOctets_custom_function_v6(interface_row);
            ipIfStatsHCOutMcastOctets = ipIfStatsOutMcastOctets_custom_function_v6(interface_row);

            rowreq_ctx = ipIfStatsTable_allocate_rowreq_ctx(NULL);
            if (rowreq_ctx == NULL) {
                snmp_log(LOG_ERR, "memory allocation failed");
                return MFD_RESOURCE_UNAVAILABLE;
            }
            if (MFD_SUCCESS != ipIfStatsTable_indexes_set(rowreq_ctx,
                                                        ipIfStatsIPVersion,
                                                        ipIfStatsIfIndex)) {
                snmp_log(LOG_ERR, "error setting indexes while loading");
                ipIfStatsTable_release_rowreq_ctx(rowreq_ctx);
                continue;
            }

            rowreq_ctx->data.ipIfStatsInReceives = ipIfStatsInReceives;
            rowreq_ctx->data.ipIfStatsHCInReceives = ipIfStatsHCInReceives;
            rowreq_ctx->data.ipIfStatsInOctets = ipIfStatsInOctets;
            rowreq_ctx->data.ipIfStatsHCInOctets = ipIfStatsHCInOctets;
            rowreq_ctx->data.ipIfStatsInHdrErrors = ipIfStatsInHdrErrors;
            rowreq_ctx->data.ipIfStatsInNoRoutes = ipIfStatsInNoRoutes;
            rowreq_ctx->data.ipIfStatsInAddrErrors = ipIfStatsInAddrErrors;
            rowreq_ctx->data.ipIfStatsInUnknownProtos = ipIfStatsInUnknownProtos;
            rowreq_ctx->data.ipIfStatsInTruncatedPkts = ipIfStatsInTruncatedPkts;
            rowreq_ctx->data.ipIfStatsInForwDatagrams = ipIfStatsInForwDatagrams;
            rowreq_ctx->data.ipIfStatsHCInForwDatagrams =
                ipIfStatsHCInForwDatagrams;
            rowreq_ctx->data.ipIfStatsReasmReqds = ipIfStatsReasmReqds;
            rowreq_ctx->data.ipIfStatsReasmOKs = ipIfStatsReasmOKs;
            rowreq_ctx->data.ipIfStatsReasmFails = ipIfStatsReasmFails;
            rowreq_ctx->data.ipIfStatsInDiscards = ipIfStatsInDiscards;
            rowreq_ctx->data.ipIfStatsInDelivers = ipIfStatsInDelivers;
            rowreq_ctx->data.ipIfStatsHCInDelivers = ipIfStatsHCInDelivers;
            rowreq_ctx->data.ipIfStatsOutRequests = ipIfStatsOutRequests;
            rowreq_ctx->data.ipIfStatsHCOutRequests = ipIfStatsHCOutRequests;
            rowreq_ctx->data.ipIfStatsOutForwDatagrams = ipIfStatsOutForwDatagrams;
            rowreq_ctx->data.ipIfStatsHCOutForwDatagrams =
                ipIfStatsHCOutForwDatagrams;
            rowreq_ctx->data.ipIfStatsOutDiscards = ipIfStatsOutDiscards;
            rowreq_ctx->data.ipIfStatsOutFragReqds = ipIfStatsOutFragReqds;
            rowreq_ctx->data.ipIfStatsOutFragOKs = ipIfStatsOutFragOKs;
            rowreq_ctx->data.ipIfStatsOutFragFails = ipIfStatsOutFragFails;
            rowreq_ctx->data.ipIfStatsOutFragCreates = ipIfStatsOutFragCreates;
            rowreq_ctx->data.ipIfStatsOutTransmits = ipIfStatsOutTransmits;
            rowreq_ctx->data.ipIfStatsHCOutTransmits = ipIfStatsHCOutTransmits;
            rowreq_ctx->data.ipIfStatsOutOctets = ipIfStatsOutOctets;
            rowreq_ctx->data.ipIfStatsHCOutOctets = ipIfStatsHCOutOctets;
            rowreq_ctx->data.ipIfStatsInMcastPkts = ipIfStatsInMcastPkts;
            rowreq_ctx->data.ipIfStatsHCInMcastPkts = ipIfStatsHCInMcastPkts;
            rowreq_ctx->data.ipIfStatsInMcastOctets = ipIfStatsInMcastOctets;
            rowreq_ctx->data.ipIfStatsHCInMcastOctets = ipIfStatsHCInMcastOctets;
            rowreq_ctx->data.ipIfStatsOutMcastPkts = ipIfStatsOutMcastPkts;
            rowreq_ctx->data.ipIfStatsHCOutMcastPkts = ipIfStatsHCOutMcastPkts;
            rowreq_ctx->data.ipIfStatsOutMcastOctets = ipIfStatsOutMcastOctets;
            rowreq_ctx->data.ipIfStatsHCOutMcastOctets = ipIfStatsHCOutMcastOctets;
            rowreq_ctx->data.ipIfStatsInBcastPkts = ipIfStatsInBcastPkts;
            rowreq_ctx->data.ipIfStatsHCInBcastPkts = ipIfStatsHCInBcastPkts;
            rowreq_ctx->data.ipIfStatsOutBcastPkts = ipIfStatsOutBcastPkts;
            rowreq_ctx->data.ipIfStatsHCOutBcastPkts = ipIfStatsHCOutBcastPkts;
            rowreq_ctx->data.ipIfStatsDiscontinuityTime =
                ipIfStatsDiscontinuityTime;
            rowreq_ctx->data.ipIfStatsRefreshRate = ipIfStatsRefreshRate;
            CONTAINER_INSERT(container, rowreq_ctx);
            ++count;
        }
        else{
            continue;
        }
    }
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsTable_container_load",
                "inserted %d records\n", (int)count));
    return MFD_SUCCESS;
}

void ipIfStatsTable_container_free(netsnmp_container *container) {
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsTable_container_free", "called\n"));
}

int ipIfStatsTable_row_prep(ipIfStatsTable_rowreq_ctx *rowreq_ctx) {
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsTable_row_prep", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);
    return MFD_SUCCESS;
}
