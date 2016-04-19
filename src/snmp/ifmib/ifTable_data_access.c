#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "IF_MIB_custom.h"
#include "ifTable.h"
#include "ifTable_data_access.h"
#include "ifTable_ovsdb_get.h"

#include "openswitch-idl.h"
#include "ovsdb-idl.h"
#include "vswitch-idl.h"
#include "openvswitch/vlog.h"

int ifTable_init_data(ifTable_registration *ifTable_reg) {
    DEBUGMSGTL(("verbose:ifTable:ifTable_init_data", "called\n"));
    return MFD_SUCCESS;
}

void ifTable_container_init(netsnmp_container **container_ptr_ptr,
                            netsnmp_cache *cache) {
    DEBUGMSGTL(("verbose:ifTable:ifTable_container_init", "called\n"));
    if (NULL == container_ptr_ptr) {
        snmp_log(LOG_ERR, "bad container param to ifTable_container_init\n");
        return;
    }
    *container_ptr_ptr = NULL;
    if (NULL == cache) {
        snmp_log(LOG_ERR, "bad cache param to ifTable_container_init\n");
        return;
    }
    cache->timeout = IFTABLE_CACHE_TIMEOUT;
}

void ifTable_container_shutdown(netsnmp_container *container_ptr) {
    DEBUGMSGTL(("verbose:ifTable:ifTable_container_shutdown", "called\n"));
    if (NULL == container_ptr) {
        snmp_log(LOG_ERR, "bad params to ifTable_container_shutdown\n");
        return;
    }
}

int ifTable_container_load(netsnmp_container *container) {
    DEBUGMSGTL(("verbose:ifTable:ifTable_container_load", "called\n"));
    ifTable_rowreq_ctx *rowreq_ctx;
    size_t count = 0;

    const struct ovsrec_interface *interface_row = NULL;

    long ifIndex;

    char ifDescr[255];
    size_t ifDescr_len;
    long ifType;
    long ifMtu;
    u_long ifSpeed;
    char ifPhysAddress[255];
    size_t ifPhysAddress_len;
    long ifAdminStatus;
    long ifOperStatus;
    long ifLastChange;
    u_long ifInOctets;
    u_long ifInUcastPkts;
    u_long ifInNUcastPkts;
    u_long ifInDiscards;
    u_long ifInErrors;
    u_long ifInUnknownProtos;
    u_long ifOutOctets;
    u_long ifOutUcastPkts;
    u_long ifOutNUcastPkts;
    u_long ifOutDiscards;
    u_long ifOutErrors;
    u_long ifOutQLen;
    oid ifSpecific[MAX_OID_LEN];
    size_t ifSpecific_len;

    interface_row = ovsrec_interface_first(idl);
    if (!interface_row) {
        snmp_log(LOG_ERR, "not able to fetch interface row");
        return -1;
    }

    OVSREC_INTERFACE_FOR_EACH(interface_row, idl) {
        if (ifTable_skip_function(idl, interface_row)) {
            continue;
        }
        ovsdb_get_ifIndex(idl, interface_row, &ifIndex);

        ovsdb_get_ifDescr(idl, interface_row, ifDescr, &ifDescr_len);
        ovsdb_get_ifType(idl, interface_row, &ifType);
        ovsdb_get_ifMtu(idl, interface_row, &ifMtu);
        ovsdb_get_ifSpeed(idl, interface_row, &ifSpeed);
        ovsdb_get_ifPhysAddress(idl, interface_row, ifPhysAddress,
                                &ifPhysAddress_len);
        ovsdb_get_ifAdminStatus(idl, interface_row, &ifAdminStatus);
        ovsdb_get_ifOperStatus(idl, interface_row, &ifOperStatus);
        ovsdb_get_ifLastChange(idl, interface_row, &ifLastChange);
        ovsdb_get_ifInOctets(idl, interface_row, &ifInOctets);
        ovsdb_get_ifInUcastPkts(idl, interface_row, &ifInUcastPkts);
        ovsdb_get_ifInNUcastPkts(idl, interface_row, &ifInNUcastPkts);
        ovsdb_get_ifInDiscards(idl, interface_row, &ifInDiscards);
        ovsdb_get_ifInErrors(idl, interface_row, &ifInErrors);
        ovsdb_get_ifInUnknownProtos(idl, interface_row, &ifInUnknownProtos);
        ovsdb_get_ifOutOctets(idl, interface_row, &ifOutOctets);
        ovsdb_get_ifOutUcastPkts(idl, interface_row, &ifOutUcastPkts);
        ovsdb_get_ifOutNUcastPkts(idl, interface_row, &ifOutNUcastPkts);
        ovsdb_get_ifOutDiscards(idl, interface_row, &ifOutDiscards);
        ovsdb_get_ifOutErrors(idl, interface_row, &ifOutErrors);
        ovsdb_get_ifOutQLen(idl, interface_row, &ifOutQLen);
        ovsdb_get_ifSpecific(idl, interface_row, ifSpecific, &ifSpecific_len);

        rowreq_ctx = ifTable_allocate_rowreq_ctx(NULL);
        if (rowreq_ctx == NULL) {
            snmp_log(LOG_ERR, "memory allocation failed");
            return MFD_RESOURCE_UNAVAILABLE;
        }
        if (MFD_SUCCESS != ifTable_indexes_set(rowreq_ctx, ifIndex)) {
            snmp_log(LOG_ERR, "error setting indexes while loading");
            ifTable_release_rowreq_ctx(rowreq_ctx);
            continue;
        }

        rowreq_ctx->data.ifDescr_len = ifDescr_len * sizeof(ifDescr[0]);
        memcpy(rowreq_ctx->data.ifDescr, ifDescr,
               ifDescr_len * sizeof(ifDescr[0]));
        rowreq_ctx->data.ifType = ifType;
        rowreq_ctx->data.ifMtu = ifMtu;
        rowreq_ctx->data.ifSpeed = ifSpeed;
        rowreq_ctx->data.ifPhysAddress_len =
            ifPhysAddress_len * sizeof(ifPhysAddress[0]);
        memcpy(rowreq_ctx->data.ifPhysAddress, ifPhysAddress,
               ifPhysAddress_len * sizeof(ifPhysAddress[0]));
        rowreq_ctx->data.ifAdminStatus = ifAdminStatus;
        rowreq_ctx->data.ifOperStatus = ifOperStatus;
        rowreq_ctx->data.ifLastChange = ifLastChange;
        rowreq_ctx->data.ifInOctets = ifInOctets;
        rowreq_ctx->data.ifInUcastPkts = ifInUcastPkts;
        rowreq_ctx->data.ifInNUcastPkts = ifInNUcastPkts;
        rowreq_ctx->data.ifInDiscards = ifInDiscards;
        rowreq_ctx->data.ifInErrors = ifInErrors;
        rowreq_ctx->data.ifInUnknownProtos = ifInUnknownProtos;
        rowreq_ctx->data.ifOutOctets = ifOutOctets;
        rowreq_ctx->data.ifOutUcastPkts = ifOutUcastPkts;
        rowreq_ctx->data.ifOutNUcastPkts = ifOutNUcastPkts;
        rowreq_ctx->data.ifOutDiscards = ifOutDiscards;
        rowreq_ctx->data.ifOutErrors = ifOutErrors;
        rowreq_ctx->data.ifOutQLen = ifOutQLen;
        rowreq_ctx->data.ifSpecific_len =
            ifSpecific_len * sizeof(ifSpecific[0]);
        memcpy(rowreq_ctx->data.ifSpecific, ifSpecific,
               ifSpecific_len * sizeof(ifSpecific[0]));
        CONTAINER_INSERT(container, rowreq_ctx);
        ++count;
    }
    DEBUGMSGTL(("verbose:ifTable:ifTable_container_load",
                "inserted %d records\n", (int)count));
    return MFD_SUCCESS;
}

void ifTable_container_free(netsnmp_container *container) {
    DEBUGMSGTL(("verbose:ifTable:ifTable_container_free", "called\n"));
}

int ifTable_row_prep(ifTable_rowreq_ctx *rowreq_ctx) {
    DEBUGMSGTL(("verbose:ifTable:ifTable_row_prep", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);
    return MFD_SUCCESS;
}
