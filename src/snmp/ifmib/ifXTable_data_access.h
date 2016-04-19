#ifndef IFXTABLE_DATA_ACCESS_H
#define IFXTABLE_DATA_ACCESS_H

extern struct ovsdb_idl *idl;

int ifXTable_init_data(ifXTable_registration *ifXTable_reg);
#define IFXTABLE_CACHE_TIMEOUT 30
void ifXTable_container_init(netsnmp_container **container_ptr_ptr,
                             netsnmp_cache *cache);
void ifXTable_container_shutdown(netsnmp_container *container_ptr);
int ifXTable_container_load(netsnmp_container *container);
void ifXTable_container_free(netsnmp_container *container);
int ifXTable_cache_load(netsnmp_container *container);
void ifXTable_cache_free(netsnmp_container *container);
int ifXTable_row_prep(ifXTable_rowreq_ctx *rowreq_ctx);
#endif