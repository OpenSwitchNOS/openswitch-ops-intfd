#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/mib_modules.h>
#include "vswitch-idl.h"
#include "ovsdb-idl.h"

#include "ifTable.h"
#include "ifTable_interface.h"
#include "ifTable_ovsdb_get.h"

const oid ifTable_oid[] = {IFTABLE_OID};
const int ifTable_oid_size = OID_LENGTH(ifTable_oid);

ifTable_registration ifTable_user_context;
void initialize_table_ifTable(void);
void shutdown_table_ifTable(void);

void init_ifTable(void) {
    DEBUGMSGTL(("verbose:ifTable:init_ifTable", "called\n"));

    ifTable_registration *user_context;
    u_long flags;

    user_context = netsnmp_create_data_list("ifTable", NULL, NULL);
    flags = 0;

    _ifTable_initialize_interface(user_context, flags);

    ifTable_ovsdb_idl_init(idl);
}

void shutdown_ifTable(void) {
    _ifTable_shutdown_interface(&ifTable_user_context);
}

int ifTable_rowreq_ctx_init(ifTable_rowreq_ctx *rowreq_ctx,
                            void *user_init_ctx) {
    DEBUGMSGTL(("verbose:ifTable:ifTable_rowreq_ctx_init", "called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    return MFD_SUCCESS;
}

void ifTable_rowreq_ctx_cleanup(ifTable_rowreq_ctx *rowreq_ctx) {
    DEBUGMSGTL(("verbose:ifTable:ifTable_rowreq_ctx_cleanup", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);
}

int ifTable_pre_request(ifTable_registration *user_context) {
    DEBUGMSGTL(("verbose:ifTable:ifTable_pre_request", "called\n"));
    return MFD_SUCCESS;
}

int ifTable_post_request(ifTable_registration *user_context, int rc) {
    DEBUGMSGTL(("verbose:ifTable:ifTable_post_request", "called\n"));
    return MFD_SUCCESS;
}
