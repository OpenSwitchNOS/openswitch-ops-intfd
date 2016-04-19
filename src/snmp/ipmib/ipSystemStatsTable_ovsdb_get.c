#include "openswitch-idl.h"
#include "ovsdb-idl.h"
#include "vswitch-idl.h"
#include "openvswitch/vlog.h"
#include "IP_MIB_custom.h"
#include "ipSystemStatsTable_ovsdb_get.h"

void ipSystemStatsTable_ovsdb_idl_init(struct ovsdb_idl *idl) {
    ovsdb_idl_add_column(idl, &ovsrec_port_col_name);
}
