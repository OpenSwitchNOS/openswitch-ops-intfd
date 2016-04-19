#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "IP_MIB_plugins.h"
#include "IP_MIB_scalars.h"
#include "ipSystemStatsTable.h"
#include "ipIfStatsTable.h"

void ops_snmp_init(void) {

    init_ipSystemStatsTable();
    init_ipIfStatsTable();
}

void ops_snmp_run(void) {}
void ops_snmp_wait(void) {}
void ops_snmp_destroy(void) {
    shutdown_ipSystemStatsTable();
    shutdown_ipIfStatsTable();
}
