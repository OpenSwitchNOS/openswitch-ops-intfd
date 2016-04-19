#ifndef IFXTABLE_INTERFACE_H
#define IFXTABLE_INTERFACE_H
#include "ifXTable.h"

void _ifXTable_initialize_interface(ifXTable_registration *user_ctx,
                                    u_long flags);
void _ifXTable_shutdown_interface(ifXTable_registration *user_ctx);
ifXTable_registration *ifXTable_registration_get(void);
ifXTable_registration *ifXTable_registration_set(ifXTable_registration *newreg);
netsnmp_container *ifXTable_container_get(void);
int ifXTable_container_size(void);
ifXTable_rowreq_ctx *ifXTable_allocate_rowreq_ctx(void *);
void ifXTable_release_rowreq_ctx(ifXTable_rowreq_ctx *rowreq_ctx);
int ifXTable_index_to_oid(netsnmp_index *oid_idx, ifXTable_mib_index *mib_idx);
int ifXTable_index_from_oid(netsnmp_index *oid_idx,
                            ifXTable_mib_index *mib_idx);
void ifXTable_valid_columns_set(netsnmp_column_info *vc);
#endif