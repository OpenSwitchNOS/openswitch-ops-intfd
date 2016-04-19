#ifndef IPSYSTEMSTATSTABLE_DATA_ACCESS_H
#define IPSYSTEMSTATSTABLE_DATA_ACCESS_H

extern struct ovsdb_idl *idl;

int ipSystemStatsTable_init_data(
    ipSystemStatsTable_registration *ipSystemStatsTable_reg);
#define IPSYSTEMSTATSTABLE_CACHE_TIMEOUT 30
void ipSystemStatsTable_container_init(netsnmp_container **container_ptr_ptr,
                                       netsnmp_cache *cache);
void ipSystemStatsTable_container_shutdown(netsnmp_container *container_ptr);
int ipSystemStatsTable_container_load(netsnmp_container *container);
void ipSystemStatsTable_container_free(netsnmp_container *container);
int ipSystemStatsTable_cache_load(netsnmp_container *container);
void ipSystemStatsTable_cache_free(netsnmp_container *container);
int ipSystemStatsTable_row_prep(ipSystemStatsTable_rowreq_ctx *rowreq_ctx);
#endif