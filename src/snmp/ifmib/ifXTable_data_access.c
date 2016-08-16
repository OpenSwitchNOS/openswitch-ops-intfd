#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "IF_MIB_custom.h"
#include "ifXTable.h"
#include "ifXTable_data_access.h"
#include "ifXTable_ovsdb_get.h"

#include "openswitch-idl.h"
#include "ovsdb-idl.h"
#include "vswitch-idl.h"
#include "openvswitch/vlog.h"

int ifXTable_init_data(ifXTable_registration *ifXTable_reg) {
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_init_data", "called\n"));
    return MFD_SUCCESS;
}

void ifXTable_container_init(netsnmp_container **container_ptr_ptr,
                             netsnmp_cache *cache) {
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_container_init", "called\n"));
    if (NULL == container_ptr_ptr) {
        snmp_log(LOG_ERR, "bad container param to ifXTable_container_init\n");
        return;
    }
    *container_ptr_ptr = NULL;
    if (NULL == cache) {
        snmp_log(LOG_ERR, "bad cache param to ifXTable_container_init\n");
        return;
    }
    cache->timeout = IFXTABLE_CACHE_TIMEOUT;
}

void ifXTable_container_shutdown(netsnmp_container *container_ptr) {
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_container_shutdown", "called\n"));
    if (NULL == container_ptr) {
        snmp_log(LOG_ERR, "bad params to ifXTable_container_shutdown\n");
        return;
    }
}

int ifXTable_container_load(netsnmp_container *container) {
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_container_load", "called\n"));
    ifXTable_rowreq_ctx *rowreq_ctx;
    size_t count = 0;

    const struct ovsrec_interface *interface_row = NULL;

    long ifIndex;

    char ifName[255];
    size_t ifName_len;
    u_long ifInMulticastPkts;
    u_long ifInBroadcastPkts;
    u_long ifOutMulticastPkts;
    u_long ifOutBroadcastPkts;
    U64 ifHCInOctets;
    U64 ifHCInUcastPkts;
    U64 ifHCInMulticastPkts;
    U64 ifHCInBroadcastPkts;
    U64 ifHCOutOctets;
    U64 ifHCOutUcastPkts;
    U64 ifHCOutMulticastPkts;
    U64 ifHCOutBroadcastPkts;
    long ifLinkUpDownTrapEnable;
    u_long ifHighSpeed;
    long ifPromiscuousMode;
    long ifConnectorPresent;
    char ifAlias[64];
    size_t ifAlias_len;
    long ifCounterDiscontinuityTime;

    interface_row = ovsrec_interface_first(idl);
    if (!interface_row) {
        snmp_log(LOG_ERR, "not able to fetch interface row");
        return -1;
    }

    OVSREC_INTERFACE_FOR_EACH(interface_row, idl) {
        if (ifXTable_skip_function(idl, interface_row)) {
            continue;
        }
        ovsdb_get_ifIndex(idl, interface_row, &ifIndex);

        ovsdb_get_ifName(idl, interface_row, ifName, &ifName_len);
        ovsdb_get_ifInMulticastPkts(idl, interface_row, &ifInMulticastPkts);
        ovsdb_get_ifInBroadcastPkts(idl, interface_row, &ifInBroadcastPkts);
        ovsdb_get_ifOutMulticastPkts(idl, interface_row, &ifOutMulticastPkts);
        ovsdb_get_ifOutBroadcastPkts(idl, interface_row, &ifOutBroadcastPkts);
        ovsdb_get_ifHCInOctets(idl, interface_row, &ifHCInOctets);
        ovsdb_get_ifHCInUcastPkts(idl, interface_row, &ifHCInUcastPkts);
        ovsdb_get_ifHCInMulticastPkts(idl, interface_row, &ifHCInMulticastPkts);
        ovsdb_get_ifHCInBroadcastPkts(idl, interface_row, &ifHCInBroadcastPkts);
        ovsdb_get_ifHCOutOctets(idl, interface_row, &ifHCOutOctets);
        ovsdb_get_ifHCOutUcastPkts(idl, interface_row, &ifHCOutUcastPkts);
        ovsdb_get_ifHCOutMulticastPkts(idl, interface_row,
                                       &ifHCOutMulticastPkts);
        ovsdb_get_ifHCOutBroadcastPkts(idl, interface_row,
                                       &ifHCOutBroadcastPkts);
        ovsdb_get_ifLinkUpDownTrapEnable(idl, interface_row,
                                         &ifLinkUpDownTrapEnable);
        ovsdb_get_ifHighSpeed(idl, interface_row, &ifHighSpeed);
        ovsdb_get_ifPromiscuousMode(idl, interface_row, &ifPromiscuousMode);
        ovsdb_get_ifConnectorPresent(idl, interface_row, &ifConnectorPresent);
        ovsdb_get_ifAlias(idl, interface_row, ifAlias, &ifAlias_len);
        ovsdb_get_ifCounterDiscontinuityTime(idl, interface_row,
                                             &ifCounterDiscontinuityTime);

        rowreq_ctx = ifXTable_allocate_rowreq_ctx(NULL);
        if (rowreq_ctx == NULL) {
            snmp_log(LOG_ERR, "memory allocation failed");
            return MFD_RESOURCE_UNAVAILABLE;
        }
        if (MFD_SUCCESS != ifXTable_indexes_set(rowreq_ctx, ifIndex)) {
            snmp_log(LOG_ERR, "error setting indexes while loading");
            ifXTable_release_rowreq_ctx(rowreq_ctx);
            continue;
        }

        rowreq_ctx->data.ifName_len = ifName_len * sizeof(ifName[0]);
        memcpy(rowreq_ctx->data.ifName, ifName, ifName_len * sizeof(ifName[0]));
        rowreq_ctx->data.ifInMulticastPkts = ifInMulticastPkts;
        rowreq_ctx->data.ifInBroadcastPkts = ifInBroadcastPkts;
        rowreq_ctx->data.ifOutMulticastPkts = ifOutMulticastPkts;
        rowreq_ctx->data.ifOutBroadcastPkts = ifOutBroadcastPkts;
        rowreq_ctx->data.ifHCInOctets = ifHCInOctets;
        rowreq_ctx->data.ifHCInUcastPkts = ifHCInUcastPkts;
        rowreq_ctx->data.ifHCInMulticastPkts = ifHCInMulticastPkts;
        rowreq_ctx->data.ifHCInBroadcastPkts = ifHCInBroadcastPkts;
        rowreq_ctx->data.ifHCOutOctets = ifHCOutOctets;
        rowreq_ctx->data.ifHCOutUcastPkts = ifHCOutUcastPkts;
        rowreq_ctx->data.ifHCOutMulticastPkts = ifHCOutMulticastPkts;
        rowreq_ctx->data.ifHCOutBroadcastPkts = ifHCOutBroadcastPkts;
        rowreq_ctx->data.ifLinkUpDownTrapEnable = ifLinkUpDownTrapEnable;
        rowreq_ctx->data.ifHighSpeed = ifHighSpeed;
        rowreq_ctx->data.ifPromiscuousMode = ifPromiscuousMode;
        rowreq_ctx->data.ifConnectorPresent = ifConnectorPresent;
        rowreq_ctx->data.ifAlias_len = ifAlias_len * sizeof(ifAlias[0]);
        memcpy(rowreq_ctx->data.ifAlias, ifAlias,
               ifAlias_len * sizeof(ifAlias[0]));
        rowreq_ctx->data.ifCounterDiscontinuityTime =
            ifCounterDiscontinuityTime;
        CONTAINER_INSERT(container, rowreq_ctx);
        ++count;
    }
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_container_load",
                "inserted %d records\n", (int)count));
    return MFD_SUCCESS;
}

void ifXTable_container_free(netsnmp_container *container) {
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_container_free", "called\n"));
}

int ifXTable_row_prep(ifXTable_rowreq_ctx *rowreq_ctx) {
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_row_prep", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);
    return MFD_SUCCESS;
}
