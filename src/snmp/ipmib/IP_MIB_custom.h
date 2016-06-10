#ifndef IP_MIB_CUSTOM_H
#define IP_MIB_CUSTOM_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "vswitch-idl.h"


int portTable_skip_function(const struct ovsdb_idl *idl,
                          const struct ovsrec_port *port_row);

int portTable_inetv4(struct ovsdb_idl *idl, const struct ovsrec_port *port_row,
                     long *ifIndex);

int portTable_inetv6(struct ovsdb_idl *idl, const struct ovsrec_port *port_row,
                     long *ifIndex);

unsigned long long ipIfStatsInReceives_custom_function_v4(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsInOctets_custom_function_v4(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsOutTransmits_custom_function_v4(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsOutOctets_custom_function_v4(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsInMcastPkts_custom_function_v4(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsInMcastOctets_custom_function_v4(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsOutMcastPkts_custom_function_v4(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsOutMcastOctets_custom_function_v4(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsInReceives_custom_function_v6(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsInOctets_custom_function_v6(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsOutTransmits_custom_function_v6(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsOutOctets_custom_function_v6(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsInMcastPkts_custom_function_v6(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsInMcastOctets_custom_function_v6(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsOutMcastPkts_custom_function_v6(
    const struct ovsrec_interface *int_row);

unsigned long long ipIfStatsOutMcastOctets_custom_function_v6(
    const struct ovsrec_interface *int_row);


#endif
