#ifndef IFTABLE_OVSDB_GET_H
#define IFTABLE_OVSDB_GET_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "vswitch-idl.h"
#include "ovsdb-idl.h"

void ifTable_ovsdb_idl_init(struct ovsdb_idl *idl);
void ovsdb_get_ifIndex(struct ovsdb_idl *idl,
                       const struct ovsrec_interface *interface_row,
                       long *ifIndex_val_ptr);

void ovsdb_get_ifDescr(struct ovsdb_idl *idl,
                       const struct ovsrec_interface *interface_row,
                       char *ifDescr_val_ptr, size_t *ifDescr_val_ptr_len);
void ovsdb_get_ifType(struct ovsdb_idl *idl,
                      const struct ovsrec_interface *interface_row,
                      long *ifType_val_ptr);
void ovsdb_get_ifMtu(struct ovsdb_idl *idl,
                     const struct ovsrec_interface *interface_row,
                     long *ifMtu_val_ptr);
void ovsdb_get_ifSpeed(struct ovsdb_idl *idl,
                       const struct ovsrec_interface *interface_row,
                       u_long *ifSpeed_val_ptr);
void ovsdb_get_ifPhysAddress(struct ovsdb_idl *idl,
                             const struct ovsrec_interface *interface_row,
                             char *ifPhysAddress_val_ptr,
                             size_t *ifPhysAddress_val_ptr_len);
void ovsdb_get_ifAdminStatus(struct ovsdb_idl *idl,
                             const struct ovsrec_interface *interface_row,
                             long *ifAdminStatus_val_ptr);
void ovsdb_get_ifOperStatus(struct ovsdb_idl *idl,
                            const struct ovsrec_interface *interface_row,
                            long *ifOperStatus_val_ptr);
void ovsdb_get_ifLastChange(struct ovsdb_idl *idl,
                            const struct ovsrec_interface *interface_row,
                            long *ifLastChange_val_ptr);
void ovsdb_get_ifInOctets(struct ovsdb_idl *idl,
                          const struct ovsrec_interface *interface_row,
                          u_long *ifInOctets_val_ptr);
void ovsdb_get_ifInUcastPkts(struct ovsdb_idl *idl,
                             const struct ovsrec_interface *interface_row,
                             u_long *ifInUcastPkts_val_ptr);
void ovsdb_get_ifInNUcastPkts(struct ovsdb_idl *idl,
                              const struct ovsrec_interface *interface_row,
                              u_long *ifInNUcastPkts_val_ptr);
void ovsdb_get_ifInDiscards(struct ovsdb_idl *idl,
                            const struct ovsrec_interface *interface_row,
                            u_long *ifInDiscards_val_ptr);
void ovsdb_get_ifInErrors(struct ovsdb_idl *idl,
                          const struct ovsrec_interface *interface_row,
                          u_long *ifInErrors_val_ptr);
void ovsdb_get_ifInUnknownProtos(struct ovsdb_idl *idl,
                                 const struct ovsrec_interface *interface_row,
                                 u_long *ifInUnknownProtos_val_ptr);
void ovsdb_get_ifOutOctets(struct ovsdb_idl *idl,
                           const struct ovsrec_interface *interface_row,
                           u_long *ifOutOctets_val_ptr);
void ovsdb_get_ifOutUcastPkts(struct ovsdb_idl *idl,
                              const struct ovsrec_interface *interface_row,
                              u_long *ifOutUcastPkts_val_ptr);
void ovsdb_get_ifOutNUcastPkts(struct ovsdb_idl *idl,
                               const struct ovsrec_interface *interface_row,
                               u_long *ifOutNUcastPkts_val_ptr);
void ovsdb_get_ifOutDiscards(struct ovsdb_idl *idl,
                             const struct ovsrec_interface *interface_row,
                             u_long *ifOutDiscards_val_ptr);
void ovsdb_get_ifOutErrors(struct ovsdb_idl *idl,
                           const struct ovsrec_interface *interface_row,
                           u_long *ifOutErrors_val_ptr);
void ovsdb_get_ifOutQLen(struct ovsdb_idl *idl,
                         const struct ovsrec_interface *interface_row,
                         u_long *ifOutQLen_val_ptr);
void ovsdb_get_ifSpecific(struct ovsdb_idl *idl,
                          const struct ovsrec_interface *interface_row,
                          oid *ifSpecific_val_ptr,
                          size_t *ifSpecific_val_ptr_len);
#endif
