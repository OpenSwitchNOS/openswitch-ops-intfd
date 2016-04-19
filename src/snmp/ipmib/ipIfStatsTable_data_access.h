#ifndef IPIFSTATSTABLE_DATA_ACCESS_H
#define IPIFSTATSTABLE_DATA_ACCESS_H

extern struct ovsdb_idl *idl;

int ipIfStatsTable_init_data(ipIfStatsTable_registration *ipIfStatsTable_reg);
#define IPIFSTATSTABLE_CACHE_TIMEOUT 30
void ipIfStatsTable_container_init(netsnmp_container **container_ptr_ptr,
                                   netsnmp_cache *cache);
void ipIfStatsTable_container_shutdown(netsnmp_container *container_ptr);
int ipIfStatsTable_container_load(netsnmp_container *container);
void ipIfStatsTable_container_free(netsnmp_container *container);
int ipIfStatsTable_cache_load(netsnmp_container *container);
void ipIfStatsTable_cache_free(netsnmp_container *container);
int ipIfStatsTable_row_prep(ipIfStatsTable_rowreq_ctx *rowreq_ctx);
#endif