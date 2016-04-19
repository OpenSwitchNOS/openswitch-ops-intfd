#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/mib_modules.h>
#include "vswitch-idl.h"
#include "ovsdb-idl.h"

#include "ipSystemStatsTable.h"
#include "ipSystemStatsTable_interface.h"
#include "ipSystemStatsTable_ovsdb_get.h"

const oid ipSystemStatsTable_oid[] = {IPSYSTEMSTATSTABLE_OID};
const int ipSystemStatsTable_oid_size = OID_LENGTH(ipSystemStatsTable_oid);

ipSystemStatsTable_registration ipSystemStatsTable_user_context;
void initialize_table_ipSystemStatsTable(void);
void shutdown_table_ipSystemStatsTable(void);

void init_ipSystemStatsTable(void) {
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:init_ipSystemStatsTable", "called\n"));

    ipSystemStatsTable_registration *user_context;
    u_long flags;

    user_context = netsnmp_create_data_list("ipSystemStatsTable", NULL, NULL);
    flags = 0;

    _ipSystemStatsTable_initialize_interface(user_context, flags);

    ipSystemStatsTable_ovsdb_idl_init(idl);
}

void shutdown_ipSystemStatsTable(void) {
    _ipSystemStatsTable_shutdown_interface(&ipSystemStatsTable_user_context);
}

int ipSystemStatsTable_rowreq_ctx_init(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx, void *user_init_ctx) {
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsTable_rowreq_ctx_init",
                "called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    return MFD_SUCCESS;
}

void ipSystemStatsTable_rowreq_ctx_cleanup(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx) {
    DEBUGMSGTL(
        ("verbose:ipSystemStatsTable:ipSystemStatsTable_rowreq_ctx_cleanup",
         "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);
}

int ipSystemStatsTable_pre_request(
    ipSystemStatsTable_registration *user_context) {
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsTable_pre_request",
                "called\n"));
    return MFD_SUCCESS;
}

int ipSystemStatsTable_post_request(
    ipSystemStatsTable_registration *user_context, int rc) {
    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsTable_post_request",
                "called\n"));
    return MFD_SUCCESS;
}
