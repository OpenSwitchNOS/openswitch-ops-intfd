#ifndef IFXTABLE_OVSDB_GET_H
#define IFXTABLE_OVSDB_GET_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "vswitch-idl.h"
#include "ovsdb-idl.h"

void ifXTable_ovsdb_idl_init(struct ovsdb_idl *idl);
void ovsdb_get_ifIndex(struct ovsdb_idl *idl,
                       const struct ovsrec_interface *interface_row,
                       long *ifIndex_val_ptr);

void ovsdb_get_ifName(struct ovsdb_idl *idl,
                      const struct ovsrec_interface *interface_row,
                      char *ifName_val_ptr, size_t *ifName_val_ptr_len);
void ovsdb_get_ifInMulticastPkts(struct ovsdb_idl *idl,
                                 const struct ovsrec_interface *interface_row,
                                 u_long *ifInMulticastPkts_val_ptr);
void ovsdb_get_ifInBroadcastPkts(struct ovsdb_idl *idl,
                                 const struct ovsrec_interface *interface_row,
                                 u_long *ifInBroadcastPkts_val_ptr);
void ovsdb_get_ifOutMulticastPkts(struct ovsdb_idl *idl,
                                  const struct ovsrec_interface *interface_row,
                                  u_long *ifOutMulticastPkts_val_ptr);
void ovsdb_get_ifOutBroadcastPkts(struct ovsdb_idl *idl,
                                  const struct ovsrec_interface *interface_row,
                                  u_long *ifOutBroadcastPkts_val_ptr);
void ovsdb_get_ifHCInOctets(struct ovsdb_idl *idl,
                            const struct ovsrec_interface *interface_row,
                            U64 *ifHCInOctets_val_ptr);
void ovsdb_get_ifHCInUcastPkts(struct ovsdb_idl *idl,
                               const struct ovsrec_interface *interface_row,
                               U64 *ifHCInUcastPkts_val_ptr);
void ovsdb_get_ifHCInMulticastPkts(
    struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    U64 *ifHCInMulticastPkts_val_ptr);
void ovsdb_get_ifHCInBroadcastPkts(
    struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    U64 *ifHCInBroadcastPkts_val_ptr);
void ovsdb_get_ifHCOutOctets(struct ovsdb_idl *idl,
                             const struct ovsrec_interface *interface_row,
                             U64 *ifHCOutOctets_val_ptr);
void ovsdb_get_ifHCOutUcastPkts(struct ovsdb_idl *idl,
                                const struct ovsrec_interface *interface_row,
                                U64 *ifHCOutUcastPkts_val_ptr);
void ovsdb_get_ifHCOutMulticastPkts(
    struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    U64 *ifHCOutMulticastPkts_val_ptr);
void ovsdb_get_ifHCOutBroadcastPkts(
    struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    U64 *ifHCOutBroadcastPkts_val_ptr);
void ovsdb_get_ifLinkUpDownTrapEnable(
    struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    long *ifLinkUpDownTrapEnable_val_ptr);
void ovsdb_get_ifHighSpeed(struct ovsdb_idl *idl,
                           const struct ovsrec_interface *interface_row,
                           u_long *ifHighSpeed_val_ptr);
void ovsdb_get_ifPromiscuousMode(struct ovsdb_idl *idl,
                                 const struct ovsrec_interface *interface_row,
                                 long *ifPromiscuousMode_val_ptr);
void ovsdb_get_ifConnectorPresent(struct ovsdb_idl *idl,
                                  const struct ovsrec_interface *interface_row,
                                  long *ifConnectorPresent_val_ptr);
void ovsdb_get_ifAlias(struct ovsdb_idl *idl,
                       const struct ovsrec_interface *interface_row,
                       char *ifAlias_val_ptr, size_t *ifAlias_val_ptr_len);
void ovsdb_get_ifCounterDiscontinuityTime(
    struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    long *ifCounterDiscontinuityTime_val_ptr);
#endif
