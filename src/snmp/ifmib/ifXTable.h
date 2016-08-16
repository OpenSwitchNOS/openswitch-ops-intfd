#ifndef IFXTABLE_H
#define IFXTABLE_H
#include <net-snmp/library/asn1.h>
#include "ifXTable_oids.h"
#include "ifXTable_enums.h"

void init_ifXTable(void);
void shutdown_ifXTable(void);

typedef netsnmp_data_list ifXTable_registration;

typedef struct ifXTable_data_s {
    char ifName[255];
    size_t ifName_len;
    u_long ifInMulticastPkts;
    u_long ifInBroadcastPkts;
    u_long ifOutMulticastPkts;
    u_long ifOutBroadcastPkts;
    U64 ifHCInOctets;
    U64 ifHCInUcastPkts;
    U64 ifHCInMulticastPkts;
    U64 ifHCInBroadcastPkts;
    U64 ifHCOutOctets;
    U64 ifHCOutUcastPkts;
    U64 ifHCOutMulticastPkts;
    U64 ifHCOutBroadcastPkts;
    long ifLinkUpDownTrapEnable;
    u_long ifHighSpeed;
    long ifPromiscuousMode;
    long ifConnectorPresent;
    char ifAlias[64];
    size_t ifAlias_len;
    long ifCounterDiscontinuityTime;
} ifXTable_data;

typedef struct ifXTable_mib_index_s { long ifIndex; } ifXTable_mib_index;

typedef struct ifXTable_rowreq_ctx_s {
    netsnmp_index oid_idx;
    oid oid_tmp[MAX_OID_LEN];
    ifXTable_mib_index tbl_idx;
    ifXTable_data data;
    u_int rowreq_flags;
    netsnmp_data_list *ifXTable_data_list;
} ifXTable_rowreq_ctx;

typedef struct ifXTable_ref_rowreq_ctx_s {
    ifXTable_rowreq_ctx *rowreq_ctx;
} ifXTable_ref_rowreq_ctx;

int ifXTable_pre_request(ifXTable_registration *user_context);
int ifXTable_post_request(ifXTable_registration *user_context, int rc);
int ifXTable_rowreq_ctx_init(ifXTable_rowreq_ctx *rowreq_ctx,
                             void *user_init_ctx);
void ifXTable_rowreq_ctx_cleanup(ifXTable_rowreq_ctx *rowreq_ctx);
ifXTable_rowreq_ctx *
ifXTable_row_find_by_mib_index(ifXTable_mib_index *mib_idx);
extern const oid ifXTable_oid[];
extern const int ifXTable_oid_size;
#include "ifXTable_interface.h"
#include "ifXTable_data_access.h"
#include "ifXTable_data_get.h"
#include "ifXTable_data_set.h"

#endif
