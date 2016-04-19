#ifndef IPSYSTEMSTATSTABLE_INTERFACE_H
#define IPSYSTEMSTATSTABLE_INTERFACE_H
#include "ipSystemStatsTable.h"

void _ipSystemStatsTable_initialize_interface(
    ipSystemStatsTable_registration *user_ctx, u_long flags);
void _ipSystemStatsTable_shutdown_interface(
    ipSystemStatsTable_registration *user_ctx);
ipSystemStatsTable_registration *ipSystemStatsTable_registration_get(void);
ipSystemStatsTable_registration *
ipSystemStatsTable_registration_set(ipSystemStatsTable_registration *newreg);
netsnmp_container *ipSystemStatsTable_container_get(void);
int ipSystemStatsTable_container_size(void);
ipSystemStatsTable_rowreq_ctx *ipSystemStatsTable_allocate_rowreq_ctx(void *);
void ipSystemStatsTable_release_rowreq_ctx(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx);
int ipSystemStatsTable_index_to_oid(netsnmp_index *oid_idx,
                                    ipSystemStatsTable_mib_index *mib_idx);
int ipSystemStatsTable_index_from_oid(netsnmp_index *oid_idx,
                                      ipSystemStatsTable_mib_index *mib_idx);
void ipSystemStatsTable_valid_columns_set(netsnmp_column_info *vc);
#endif