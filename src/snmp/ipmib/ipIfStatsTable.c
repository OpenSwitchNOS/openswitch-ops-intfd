#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/mib_modules.h>
#include "vswitch-idl.h"
#include "ovsdb-idl.h"

#include "ipIfStatsTable.h"
#include "ipIfStatsTable_interface.h"
#include "ipIfStatsTable_ovsdb_get.h"

const oid ipIfStatsTable_oid[] = {IPIFSTATSTABLE_OID};
const int ipIfStatsTable_oid_size = OID_LENGTH(ipIfStatsTable_oid);

ipIfStatsTable_registration ipIfStatsTable_user_context;
void initialize_table_ipIfStatsTable(void);
void shutdown_table_ipIfStatsTable(void);

void init_ipIfStatsTable(void) {
    DEBUGMSGTL(("verbose:ipIfStatsTable:init_ipIfStatsTable", "called\n"));

    ipIfStatsTable_registration *user_context;
    u_long flags;

    user_context = netsnmp_create_data_list("ipIfStatsTable", NULL, NULL);
    flags = 0;

    _ipIfStatsTable_initialize_interface(user_context, flags);

    ipIfStatsTable_ovsdb_idl_init(idl);
}

void shutdown_ipIfStatsTable(void) {
    _ipIfStatsTable_shutdown_interface(&ipIfStatsTable_user_context);
}

int ipIfStatsTable_rowreq_ctx_init(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                   void *user_init_ctx) {
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsTable_rowreq_ctx_init", "called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    return MFD_SUCCESS;
}

void ipIfStatsTable_rowreq_ctx_cleanup(ipIfStatsTable_rowreq_ctx *rowreq_ctx) {
    DEBUGMSGTL(("verbose:ipIfStatsTable:ipIfStatsTable_rowreq_ctx_cleanup",
                "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);
}

int ipIfStatsTable_pre_request(ipIfStatsTable_registration *user_context) {
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsTable_pre_request", "called\n"));
    return MFD_SUCCESS;
}

int ipIfStatsTable_post_request(ipIfStatsTable_registration *user_context,
                                int rc) {
    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsTable_post_request", "called\n"));
    return MFD_SUCCESS;
}
