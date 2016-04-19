#ifndef IFTABLE_DATA_ACCESS_H
#define IFTABLE_DATA_ACCESS_H

extern struct ovsdb_idl *idl;

int ifTable_init_data(ifTable_registration *ifTable_reg);
#define IFTABLE_CACHE_TIMEOUT 30
void ifTable_container_init(netsnmp_container **container_ptr_ptr,
                            netsnmp_cache *cache);
void ifTable_container_shutdown(netsnmp_container *container_ptr);
int ifTable_container_load(netsnmp_container *container);
void ifTable_container_free(netsnmp_container *container);
int ifTable_cache_load(netsnmp_container *container);
void ifTable_cache_free(netsnmp_container *container);
int ifTable_row_prep(ifTable_rowreq_ctx *rowreq_ctx);
#endif