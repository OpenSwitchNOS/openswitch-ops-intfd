#include "openswitch-idl.h"
#include "ovsdb-idl.h"
#include "vswitch-idl.h"
#include "openvswitch/vlog.h"
#include "IF_MIB_custom.h"
#include "ifTable_ovsdb_get.h"

VLOG_DEFINE_THIS_MODULE (ifmib_snmp);

void ifTable_ovsdb_idl_init(struct ovsdb_idl *idl) {
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_statistics);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_hw_intf_info);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_mac_in_use);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_admin_state);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_user_config);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_mtu);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_name);
}

void ovsdb_get_ifIndex(struct ovsdb_idl *idl,
                       const struct ovsrec_interface *interface_row,
                       long *ifIndex_val_ptr) {
    ifTableifIndex_custom_function(idl, interface_row, ifIndex_val_ptr);
}

void ovsdb_get_ifDescr(struct ovsdb_idl *idl,
                       const struct ovsrec_interface *interface_row,
                       char *ifDescr_val_ptr, size_t *ifDescr_val_ptr_len) {
    *ifDescr_val_ptr = '\0';
    *ifDescr_val_ptr_len = 0;
}

void ovsdb_get_ifType(struct ovsdb_idl *idl,
                      const struct ovsrec_interface *interface_row,
                      long *ifType_val_ptr) {
    *ifType_val_ptr = (long)NULL;
}

void ovsdb_get_ifMtu(struct ovsdb_idl *idl,
                     const struct ovsrec_interface *interface_row,
                     long *ifMtu_val_ptr) {
    const struct ovsdb_datum *datum;

    datum = ovsrec_interface_get_mtu(interface_row, OVSDB_TYPE_INTEGER);
    if ((NULL!=datum) && (datum->n >0)) {
        *ifMtu_val_ptr = (long)datum->keys[0].integer;
    }
    else {
        *ifMtu_val_ptr = 0;
    }
}

void ovsdb_get_ifSpeed(struct ovsdb_idl *idl,
                       const struct ovsrec_interface *interface_row,
                       u_long *ifSpeed_val_ptr) {
    char *temp = (char *)smap_get(&interface_row->hw_intf_info, "max_speed");
    if (temp == NULL) {
        *ifSpeed_val_ptr = 0;
    } else {
        *ifSpeed_val_ptr = (u_long)atoi(temp);
    }
}

void ovsdb_get_ifPhysAddress(struct ovsdb_idl *idl,
                             const struct ovsrec_interface *interface_row,
                             char *ifPhysAddress_val_ptr,
                             size_t *ifPhysAddress_val_ptr_len) {
    ifPhysAddress_custom_function(idl, interface_row, ifPhysAddress_val_ptr,
                                  ifPhysAddress_val_ptr_len);
}

void ovsdb_get_ifAdminStatus(struct ovsdb_idl *idl,
                             const struct ovsrec_interface *interface_row,
                             long *ifAdminStatus_val_ptr) {
    ifAdminStatus_custom_function(idl, interface_row, ifAdminStatus_val_ptr);
}

void ovsdb_get_ifOperStatus(struct ovsdb_idl *idl,
                            const struct ovsrec_interface *interface_row,
                            long *ifOperStatus_val_ptr) {
    ifOperStatus_custom_function(idl, interface_row, ifOperStatus_val_ptr);
}

void ovsdb_get_ifLastChange(struct ovsdb_idl *idl,
                            const struct ovsrec_interface *interface_row,
                            long *ifLastChange_val_ptr) {
    *ifLastChange_val_ptr = (long)NULL;
}

void ovsdb_get_ifInOctets(struct ovsdb_idl *idl,
                          const struct ovsrec_interface *interface_row,
                          u_long *ifInOctets_val_ptr) {
    ifInOctets_custom_function(idl, interface_row, ifInOctets_val_ptr);
}

void ovsdb_get_ifInUcastPkts(struct ovsdb_idl *idl,
                             const struct ovsrec_interface *interface_row,
                             u_long *ifInUcastPkts_val_ptr) {
    ifInUcastPkts_custom_function(idl, interface_row, ifInUcastPkts_val_ptr);
}

void ovsdb_get_ifInNUcastPkts(struct ovsdb_idl *idl,
                              const struct ovsrec_interface *interface_row,
                              u_long *ifInNUcastPkts_val_ptr) {
    ifInNUcastPkts_custom_function(idl, interface_row, ifInNUcastPkts_val_ptr);
}

void ovsdb_get_ifInDiscards(struct ovsdb_idl *idl,
                            const struct ovsrec_interface *interface_row,
                            u_long *ifInDiscards_val_ptr) {
    ifInDiscards_custom_function(idl, interface_row, ifInDiscards_val_ptr);
}

void ovsdb_get_ifInErrors(struct ovsdb_idl *idl,
                          const struct ovsrec_interface *interface_row,
                          u_long *ifInErrors_val_ptr) {
    ifInErrors_custom_function(idl, interface_row, ifInErrors_val_ptr);
}

void ovsdb_get_ifInUnknownProtos(struct ovsdb_idl *idl,
                                 const struct ovsrec_interface *interface_row,
                                 u_long *ifInUnknownProtos_val_ptr) {
    ifInUnknownProtos_custom_function(idl, interface_row,
                                      ifInUnknownProtos_val_ptr);
}

void ovsdb_get_ifOutOctets(struct ovsdb_idl *idl,
                           const struct ovsrec_interface *interface_row,
                           u_long *ifOutOctets_val_ptr) {
    ifOutOctets_custom_function(idl, interface_row, ifOutOctets_val_ptr);
}

void ovsdb_get_ifOutUcastPkts(struct ovsdb_idl *idl,
                              const struct ovsrec_interface *interface_row,
                              u_long *ifOutUcastPkts_val_ptr) {
    ifOutUcastPkts_custom_function(idl, interface_row, ifOutUcastPkts_val_ptr);
}

void ovsdb_get_ifOutNUcastPkts(struct ovsdb_idl *idl,
                               const struct ovsrec_interface *interface_row,
                               u_long *ifOutNUcastPkts_val_ptr) {
    ifOutNUcastPkts_custom_function(idl, interface_row,
                                    ifOutNUcastPkts_val_ptr);
}

void ovsdb_get_ifOutDiscards(struct ovsdb_idl *idl,
                             const struct ovsrec_interface *interface_row,
                             u_long *ifOutDiscards_val_ptr) {
    ifOutDiscards_custom_function(idl, interface_row, ifOutDiscards_val_ptr);
}

void ovsdb_get_ifOutErrors(struct ovsdb_idl *idl,
                           const struct ovsrec_interface *interface_row,
                           u_long *ifOutErrors_val_ptr) {
    ifOutErrors_custom_function(idl, interface_row, ifOutErrors_val_ptr);
}

void ovsdb_get_ifOutQLen(struct ovsdb_idl *idl,
                         const struct ovsrec_interface *interface_row,
                         u_long *ifOutQLen_val_ptr) {
    *ifOutQLen_val_ptr = (u_long)NULL;
}

void ovsdb_get_ifSpecific(struct ovsdb_idl *idl,
                          const struct ovsrec_interface *interface_row,
                          oid *ifSpecific_val_ptr,
                          size_t *ifSpecific_val_ptr_len) {
    *ifSpecific_val_ptr = (oid)NULL;
    *ifSpecific_val_ptr_len = 0;
}
