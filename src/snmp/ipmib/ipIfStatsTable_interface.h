#ifndef IPIFSTATSTABLE_INTERFACE_H
#define IPIFSTATSTABLE_INTERFACE_H
#include "ipIfStatsTable.h"

void _ipIfStatsTable_initialize_interface(ipIfStatsTable_registration *user_ctx,
                                          u_long flags);
void _ipIfStatsTable_shutdown_interface(ipIfStatsTable_registration *user_ctx);
ipIfStatsTable_registration *ipIfStatsTable_registration_get(void);
ipIfStatsTable_registration *
ipIfStatsTable_registration_set(ipIfStatsTable_registration *newreg);
netsnmp_container *ipIfStatsTable_container_get(void);
int ipIfStatsTable_container_size(void);
ipIfStatsTable_rowreq_ctx *ipIfStatsTable_allocate_rowreq_ctx(void *);
void ipIfStatsTable_release_rowreq_ctx(ipIfStatsTable_rowreq_ctx *rowreq_ctx);
int ipIfStatsTable_index_to_oid(netsnmp_index *oid_idx,
                                ipIfStatsTable_mib_index *mib_idx);
int ipIfStatsTable_index_from_oid(netsnmp_index *oid_idx,
                                  ipIfStatsTable_mib_index *mib_idx);
void ipIfStatsTable_valid_columns_set(netsnmp_column_info *vc);
#endif