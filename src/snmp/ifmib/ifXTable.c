#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/mib_modules.h>
#include "vswitch-idl.h"
#include "ovsdb-idl.h"

#include "ifXTable.h"
#include "ifXTable_interface.h"
#include "ifXTable_ovsdb_get.h"

const oid ifXTable_oid[] = {IFXTABLE_OID};
const int ifXTable_oid_size = OID_LENGTH(ifXTable_oid);

ifXTable_registration ifXTable_user_context;
void initialize_table_ifXTable(void);
void shutdown_table_ifXTable(void);

void init_ifXTable(void) {
    DEBUGMSGTL(("verbose:ifXTable:init_ifXTable", "called\n"));

    ifXTable_registration *user_context;
    u_long flags;

    user_context = netsnmp_create_data_list("ifXTable", NULL, NULL);
    flags = 0;

    _ifXTable_initialize_interface(user_context, flags);

    ifXTable_ovsdb_idl_init(idl);
}

void shutdown_ifXTable(void) {
    _ifXTable_shutdown_interface(&ifXTable_user_context);
}

int ifXTable_rowreq_ctx_init(ifXTable_rowreq_ctx *rowreq_ctx,
                             void *user_init_ctx) {
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_rowreq_ctx_init", "called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    return MFD_SUCCESS;
}

void ifXTable_rowreq_ctx_cleanup(ifXTable_rowreq_ctx *rowreq_ctx) {
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_rowreq_ctx_cleanup", "called\n"));
    netsnmp_assert(NULL != rowreq_ctx);
}

int ifXTable_pre_request(ifXTable_registration *user_context) {
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_pre_request", "called\n"));
    return MFD_SUCCESS;
}

int ifXTable_post_request(ifXTable_registration *user_context, int rc) {
    DEBUGMSGTL(("verbose:ifXTable:ifXTable_post_request", "called\n"));
    return MFD_SUCCESS;
}
