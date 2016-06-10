#include "vswitch-idl.h"
#include "ovsdb-idl.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#define MAX_ADMIN_STATE_LENGTH 8
#define MAC_ADDRESS_OCTATES 6
int ifTable_skip_function(const struct ovsdb_idl *idl,
                          const struct ovsrec_interface *interface_row);

int ifXTable_skip_function(const struct ovsdb_idl *idl,
                           const struct ovsrec_interface *interface_row);

void ifTableifIndex_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    long *ifIndex_val_ptr);

void ifDescr_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    char *ifDescr_val_ptr, size_t *ifDescr_val_ptr_len);

void ifType_custom_function(const struct ovsdb_idl *idl,
                            const struct ovsrec_interface *interface_row,
                            long *ifType_val_ptr);


void ifPhysAddress_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    char *ifPhysAddress_val_ptr, size_t *ifPhysAddress_val_ptr_len);


void ifAdminStatus_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    long *ifAdminStatus_val_ptr);

void ifOperStatus_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row, long *ifOperStatus_val_ptr);

void ifLastChange_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    long *ifLastChange_val_ptr);

void ifInOctets_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    unsigned long *ifInOctets_val_ptr);

void ifInUcastPkts_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    unsigned long *ifInUcastPkts_val_ptr);

void ifInNUcastPkts_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    unsigned long *ifInNUcastPkts_val_ptr);

void ifInDiscards_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    unsigned long *ifInDiscards_val_ptr);

void ifInErrors_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    unsigned long *ifInErrors_val_ptr);

void ifInUnknownProtos_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    unsigned long * ifInUnknownProtos_val_ptr);

void ifOutOctets_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    unsigned long *ifOutOctets_val_ptr);

void ifOutUcastPkts_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    unsigned long *ifOutUcastPkts_val_ptr);

void ifOutNUcastPkts_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    unsigned long *ifOutNUcastPkts_val_ptr);

void ifOutDiscards_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row,
    unsigned long *ifOutDiscards_val_ptr);

void ifOutErrors_custom_function(
     const struct ovsdb_idl *idl,
     const struct ovsrec_interface *interface_row,
     unsigned long *ifOutErrors_val_ptr);

void ifOutQLen_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row, long *ifOutQLen_val_ptr);

void ifSpecific_custom_function(
    const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row, oid *ifSpecific_val_ptr,
    size_t *ifSpecific_val_ptr_len);

void ifInMulticastPkts_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long *ifInMulticastPkts_val_ptr);

void ifInBroadcastPkts_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long *ifInBroadcastPkts_val_ptr);

void ifOutMulticastPkts_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long *ifOutMulticastPkts_val_ptr);

void ifOutBroadcastPkts_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long *ifOutBroadcastPkts_val_ptr);

void ifHCInOctets_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long long *ifHCInOctets_val_ptr);

void ifHCInUcastPkts_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long long *ifHCInUcastPkts_val_ptr);

void ifHCInMulticastPkts_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long long *ifHCInMulticastPkts_val_ptr);

void ifHCInBroadcastPkts_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long long *ifHCInBroadcastPkts_val_ptr);

void ifHCOutOctets_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long long *ifHCOutOctets_val_ptr);

void ifHCOutUcastPkts_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long long *ifHCOutUcastPkts_val_ptr);

void ifHCOutMulticastPkts_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long long *ifHCOutMulticastPkts_val_ptr);

void ifHCOutBroadcastPkts_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long long *ifHCOutBroadcastPkts_val_ptr);

void ifLinkUpDownTrapEnable_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    long *ifLinkUpDownTrapEnable_val_ptr);

void ifHighSpeed_custom_function(const struct ovsdb_idl *idl,
    const struct ovsrec_interface *interface_row, long *ifHighSpeed_val_ptr);

void ifPromiscuousMode_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    long *ifPromiscuousMode_val_ptr);

void ifConnectorPresent_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    long *ifConnectorPresent_val_ptr);

void ifAlias_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    char *ifAlias_val_ptr, size_t *ifAlias_val_ptr_len);

void ifCounterDiscontinuityTime_custom_function(
    const struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    long *ifCounterDiscontinuityTime_val_ptr);
