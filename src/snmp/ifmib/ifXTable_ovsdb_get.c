#include "openswitch-idl.h"
#include "ovsdb-idl.h"
#include "vswitch-idl.h"
#include "openvswitch/vlog.h"
#include "IF_MIB_custom.h"
#include "ifXTable_ovsdb_get.h"

void ifXTable_ovsdb_idl_init(struct ovsdb_idl *idl) {
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_statistics);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_pm_info);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_name);
}

void ovsdb_get_ifName(struct ovsdb_idl *idl,
                      const struct ovsrec_interface *interface_row,
                      char *ifName_val_ptr, size_t *ifName_val_ptr_len) {
    char *temp = (char *)interface_row->name;
    *ifName_val_ptr_len = temp != NULL ? strlen(temp) : 0;
    memcpy(ifName_val_ptr, temp, *ifName_val_ptr_len);
}

void ovsdb_get_ifInMulticastPkts(struct ovsdb_idl *idl,
                                 const struct ovsrec_interface *interface_row,
                                 u_long *ifInMulticastPkts_val_ptr) {
    ifInMulticastPkts_custom_function(idl, interface_row,
                                      ifInMulticastPkts_val_ptr);
}

void ovsdb_get_ifInBroadcastPkts(struct ovsdb_idl *idl,
                                 const struct ovsrec_interface *interface_row,
                                 u_long *ifInBroadcastPkts_val_ptr) {
    ifInBroadcastPkts_custom_function(idl, interface_row,
                                      ifInBroadcastPkts_val_ptr);
}

void ovsdb_get_ifOutMulticastPkts(struct ovsdb_idl *idl,
                                  const struct ovsrec_interface *interface_row,
                                  u_long *ifOutMulticastPkts_val_ptr) {
    ifOutMulticastPkts_custom_function(idl, interface_row,
                                       ifOutMulticastPkts_val_ptr);
}

void ovsdb_get_ifOutBroadcastPkts(struct ovsdb_idl *idl,
                                  const struct ovsrec_interface *interface_row,
                                  u_long *ifOutBroadcastPkts_val_ptr) {
    ifOutBroadcastPkts_custom_function(idl, interface_row,
                                       ifOutBroadcastPkts_val_ptr);
}

void ovsdb_get_ifHCInOctets(struct ovsdb_idl *idl,
                            const struct ovsrec_interface *interface_row,
                            unsigned long long *ifHCInOctets_val_ptr) {
    ifHCInOctets_custom_function(idl, interface_row, ifHCInOctets_val_ptr);
}

void ovsdb_get_ifHCInUcastPkts(struct ovsdb_idl *idl,
                               const struct ovsrec_interface *interface_row,
                               unsigned long long *ifHCInUcastPkts_val_ptr) {
    ifHCInUcastPkts_custom_function(idl, interface_row,
                                    ifHCInUcastPkts_val_ptr);
}

void ovsdb_get_ifHCInMulticastPkts(
    struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long long *ifHCInMulticastPkts_val_ptr) {
    ifHCInMulticastPkts_custom_function(idl, interface_row,
                                        ifHCInMulticastPkts_val_ptr);
}

void ovsdb_get_ifHCInBroadcastPkts(
    struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long long *ifHCInBroadcastPkts_val_ptr) {
    ifHCInBroadcastPkts_custom_function(idl, interface_row,
                                        ifHCInBroadcastPkts_val_ptr);
}

void ovsdb_get_ifHCOutOctets(struct ovsdb_idl *idl,
                             const struct ovsrec_interface *interface_row,
                             unsigned long long *ifHCOutOctets_val_ptr) {
    ifHCOutOctets_custom_function(idl, interface_row, ifHCOutOctets_val_ptr);
}

void ovsdb_get_ifHCOutUcastPkts(struct ovsdb_idl *idl,
                                const struct ovsrec_interface *interface_row,
                                unsigned long long *ifHCOutUcastPkts_val_ptr) {
    ifHCOutUcastPkts_custom_function(idl, interface_row,
                                     ifHCOutUcastPkts_val_ptr);
}

void ovsdb_get_ifHCOutMulticastPkts(
    struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long long *ifHCOutMulticastPkts_val_ptr) {
    ifHCOutMulticastPkts_custom_function(idl, interface_row,
                                         ifHCOutMulticastPkts_val_ptr);
}

void ovsdb_get_ifHCOutBroadcastPkts(
    struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    unsigned long long *ifHCOutBroadcastPkts_val_ptr) {
    ifHCOutBroadcastPkts_custom_function(idl, interface_row,
                                         ifHCOutBroadcastPkts_val_ptr);
}

void ovsdb_get_ifLinkUpDownTrapEnable(
    struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    long *ifLinkUpDownTrapEnable_val_ptr) {
    *ifLinkUpDownTrapEnable_val_ptr = (long)NULL;
}

void ovsdb_get_ifHighSpeed(struct ovsdb_idl *idl,
                           const struct ovsrec_interface *interface_row,
                           u_long *ifHighSpeed_val_ptr) {
    *ifHighSpeed_val_ptr = (u_long)NULL;
}

void ovsdb_get_ifPromiscuousMode(struct ovsdb_idl *idl,
                                 const struct ovsrec_interface *interface_row,
                                 long *ifPromiscuousMode_val_ptr) {
    *ifPromiscuousMode_val_ptr = (long)NULL;
}

void ovsdb_get_ifConnectorPresent(struct ovsdb_idl *idl,
                                  const struct ovsrec_interface *interface_row,
                                  long *ifConnectorPresent_val_ptr) {
    ifConnectorPresent_custom_function(idl, interface_row,
                                       ifConnectorPresent_val_ptr);
}

void ovsdb_get_ifAlias(struct ovsdb_idl *idl,
                       const struct ovsrec_interface *interface_row,
                       char *ifAlias_val_ptr, size_t *ifAlias_val_ptr_len) {
    *ifAlias_val_ptr = '\0';
    *ifAlias_val_ptr_len = 0;
}

void ovsdb_get_ifCounterDiscontinuityTime(
    struct ovsdb_idl *idl, const struct ovsrec_interface *interface_row,
    long *ifCounterDiscontinuityTime_val_ptr) {
    *ifCounterDiscontinuityTime_val_ptr = (long)NULL;
}
