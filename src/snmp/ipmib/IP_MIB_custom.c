#include <stdio.h>
#include <stdlib.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "vswitch-idl.h"
#include "IP_MIB_custom.h"


int portTable_skip_function(const struct ovsdb_idl *idl,
                          const struct ovsrec_port *port_row) {
    if (atoi(port_row->name) == 0)
        return 1;
    return 0;
}


int portTable_inetv4(struct ovsdb_idl *idl, const struct ovsrec_port *port_row,
                     long *ifIndex){
    int ret = 0;
    if(ifIndex != NULL){
        *ifIndex = 0;
    }

    int temp = atoi(port_row->name);
    if(temp != 0){
        if(port_row->ip4_address != NULL ||
           port_row->n_ip4_address_secondary > 0){
            if(*port_row->ip4_address != '\0'){
                if(ifIndex != NULL){
                    *ifIndex = temp;
                }
                ret = 1;
            }
        }
    }

    return ret;
}

int portTable_inetv6(struct ovsdb_idl *idl,
                     const struct ovsrec_port *port_row, long *ifIndex){
    int ret = 0;
    if(ifIndex != NULL){
        *ifIndex = 0;
    }

    int temp = atoi(port_row->name);
    if(temp != 0){
        if(port_row->ip6_address != NULL ||
           port_row->n_ip6_address_secondary > 0){
            if(*port_row->ip6_address != '\0'){
                if(ifIndex != NULL){
                    *ifIndex = temp;
                }
                ret = 1;
            }
        }
    }

    return ret;
}

unsigned long long ipIfStatsInReceives_custom_function_v4(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv4_uc_rx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv4_mc_rx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsInOctets_custom_function_v4(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv4_uc_rx_bytes") == 0){
            ret += *(interface_row->value_statistics + i);
        }
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv4_mc_rx_bytes") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsOutTransmits_custom_function_v4(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv4_uc_tx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv4_mc_tx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsOutOctets_custom_function_v4(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv4_uc_tx_bytes") == 0){
            ret += *(interface_row->value_statistics + i);
        }
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv4_mc_tx_bytes") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsInMcastPkts_custom_function_v4(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv4_mc_rx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsInMcastOctets_custom_function_v4(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv4_mc_rx_bytes") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsOutMcastPkts_custom_function_v4(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv4_mc_tx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsOutMcastOctets_custom_function_v4(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv4_mc_tx_bytes") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsInReceives_custom_function_v6(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv6_uc_rx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv6_mc_rx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsInOctets_custom_function_v6(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv6_uc_rx_bytes") == 0){
            ret += *(interface_row->value_statistics + i);
        }
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv6_mc_rx_bytes") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsOutTransmits_custom_function_v6(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv6_uc_tx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv6_mc_tx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsOutOctets_custom_function_v6(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv6_uc_tx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv6_mc_tx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsInMcastPkts_custom_function_v6(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv6_mc_rx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsInMcastOctets_custom_function_v6(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv6_mc_rx_bytes") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsOutMcastPkts_custom_function_v6(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv6_mc_tx_packets") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}

unsigned long long ipIfStatsOutMcastOctets_custom_function_v6(
    const struct ovsrec_interface *interface_row){
    unsigned long long ret = 0;

    int i = 0;
    for(i = 0;i < interface_row->n_statistics;i++){
        if(strcmp(*(interface_row->key_statistics + i),
                  "ipv6_mc_tx_bytes") == 0){
            ret += *(interface_row->value_statistics + i);
        }
    }

    return ret;
}
