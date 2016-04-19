#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "IF_MIB_plugins.h"
#include "IF_MIB_scalars.h"
#include "ifTable.h"
#include "ifXTable.h"

void ops_snmp_init(void) {

init_ifTable();
init_ifXTable();
}

void ops_snmp_run(void){}
void ops_snmp_wait(void){}
void ops_snmp_destroy(void){
shutdown_ifTable();
shutdown_ifXTable();
}
