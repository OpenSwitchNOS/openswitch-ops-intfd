#ifndef IPIFSTATSTABLE_OVSDB_GET_H
#define IPIFSTATSTABLE_OVSDB_GET_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "vswitch-idl.h"
#include "ovsdb-idl.h"

void ipIfStatsTable_ovsdb_idl_init(struct ovsdb_idl *idl);

#endif
