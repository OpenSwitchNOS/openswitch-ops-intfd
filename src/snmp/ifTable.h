#ifndef IFTABLE_H
#define IFTABLE_H
#include <net-snmp/library/asn1.h>
#include "ifTable_oids.h"
#include "ifTable_enums.h"

void init_ifTable(void);
void shutdown_ifTable(void);

typedef netsnmp_data_list ifTable_registration;

typedef struct ifTable_data_s {
    char ifDescr[255];
    size_t ifDescr_len;
    long ifType;
    long ifMtu;
    u_long ifSpeed;
    char ifPhysAddress[255];
    size_t ifPhysAddress_len;
    long ifAdminStatus;
    long ifOperStatus;
    long ifLastChange;
    u_long ifInOctets;
    u_long ifInUcastPkts;
    u_long ifInNUcastPkts;
    u_long ifInDiscards;
    u_long ifInErrors;
    u_long ifInUnknownProtos;
    u_long ifOutOctets;
    u_long ifOutUcastPkts;
    u_long ifOutNUcastPkts;
    u_long ifOutDiscards;
    u_long ifOutErrors;
    u_long ifOutQLen;
    oid ifSpecific[MAX_OID_LEN];
    size_t ifSpecific_len;
} ifTable_data;

typedef struct ifTable_mib_index_s { long ifIndex; } ifTable_mib_index;

typedef struct ifTable_rowreq_ctx_s {
    netsnmp_index oid_idx;
    oid oid_tmp[MAX_OID_LEN];
    ifTable_mib_index tbl_idx;
    ifTable_data data;
    u_int rowreq_flags;
    netsnmp_data_list *ifTable_data_list;
} ifTable_rowreq_ctx;

typedef struct ifTable_ref_rowreq_ctx_s {
    ifTable_rowreq_ctx *rowreq_ctx;
} ifTable_ref_rowreq_ctx;

int ifTable_pre_request(ifTable_registration *user_context);
int ifTable_post_request(ifTable_registration *user_context, int rc);
int ifTable_rowreq_ctx_init(ifTable_rowreq_ctx *rowreq_ctx,
                            void *user_init_ctx);
void ifTable_rowreq_ctx_cleanup(ifTable_rowreq_ctx *rowreq_ctx);
ifTable_rowreq_ctx *ifTable_row_find_by_mib_index(ifTable_mib_index *mib_idx);
extern const oid ifTable_oid[];
extern const int ifTable_oid_size;
#include "ifTable_interface.h"
#include "ifTable_data_access.h"
#include "ifTable_data_get.h"
#include "ifTable_data_set.h"

#endif