#ifndef IFTABLE_INTERFACE_H
#define IFTABLE_INTERFACE_H
#include "ifTable.h"

void _ifTable_initialize_interface(ifTable_registration *user_ctx,
                                   u_long flags);
void _ifTable_shutdown_interface(ifTable_registration *user_ctx);
ifTable_registration *ifTable_registration_get(void);
ifTable_registration *ifTable_registration_set(ifTable_registration *newreg);
netsnmp_container *ifTable_container_get(void);
int ifTable_container_size(void);
ifTable_rowreq_ctx *ifTable_allocate_rowreq_ctx(void *);
void ifTable_release_rowreq_ctx(ifTable_rowreq_ctx *rowreq_ctx);
int ifTable_index_to_oid(netsnmp_index *oid_idx, ifTable_mib_index *mib_idx);
int ifTable_index_from_oid(netsnmp_index *oid_idx, ifTable_mib_index *mib_idx);
void ifTable_valid_columns_set(netsnmp_column_info *vc);
#endif