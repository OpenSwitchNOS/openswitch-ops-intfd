#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/table_container.h>
#include <net-snmp/library/container.h>
#include "ipIfStatsTable.h"
#include "ipIfStatsTable_interface.h"

netsnmp_feature_require(baby_steps) netsnmp_feature_require(row_merge)
    netsnmp_feature_require(check_all_requests_error)

        typedef struct ipIfStatsTable_interface_ctx_s {
    netsnmp_container *container;
    netsnmp_cache *cache;
    ipIfStatsTable_registration *user_ctx;
    netsnmp_table_registration_info tbl_info;
    netsnmp_baby_steps_access_methods access_multiplexer;
} ipIfStatsTable_interface_ctx;

static ipIfStatsTable_interface_ctx ipIfStatsTable_if_ctx;
static void
_ipIfStatsTable_container_init(ipIfStatsTable_interface_ctx *if_ctx);
static void
_ipIfStatsTable_container_shutdown(ipIfStatsTable_interface_ctx *if_ctx);

netsnmp_container *ipIfStatsTable_container_get(void) {
    return ipIfStatsTable_if_ctx.container;
}

ipIfStatsTable_registration *ipIfStatsTable_registration_get(void) {
    return ipIfStatsTable_if_ctx.user_ctx;
}

ipIfStatsTable_registration *
ipIfStatsTable_registration_set(ipIfStatsTable_registration *newreg) {
    ipIfStatsTable_registration *old = ipIfStatsTable_if_ctx.user_ctx;
    ipIfStatsTable_if_ctx.user_ctx = newreg;
    return old;
}

int ipIfStatsTable_container_size(void) {
    return CONTAINER_SIZE(ipIfStatsTable_if_ctx.container);
}

static Netsnmp_Node_Handler _mfd_ipIfStatsTable_pre_request;
static Netsnmp_Node_Handler _mfd_ipIfStatsTable_post_request;
static Netsnmp_Node_Handler _mfd_ipIfStatsTable_object_lookup;
static Netsnmp_Node_Handler _mfd_ipIfStatsTable_get_values;

void _ipIfStatsTable_initialize_interface(ipIfStatsTable_registration *reg_ptr,
                                          u_long flags) {
    netsnmp_baby_steps_access_methods *access_multiplexer =
        &ipIfStatsTable_if_ctx.access_multiplexer;
    netsnmp_table_registration_info *tbl_info = &ipIfStatsTable_if_ctx.tbl_info;
    netsnmp_handler_registration *reginfo;
    netsnmp_mib_handler *handler;
    int mfd_modes = 0;

    DEBUGMSGTL(("internal:ipIfStatsTable:_ipIfStatsTable_initialize_interface",
                "called\n"));

    netsnmp_table_helper_add_indexes(tbl_info, ASN_INTEGER, ASN_INTEGER, 0);

    tbl_info->min_column = IPIFSTATSTABLE_MIN_COL;
    tbl_info->max_column = IPIFSTATSTABLE_MAX_COL;
    ipIfStatsTable_if_ctx.user_ctx = reg_ptr;
    ipIfStatsTable_init_data(reg_ptr);
    _ipIfStatsTable_container_init(&ipIfStatsTable_if_ctx);
    if (NULL == ipIfStatsTable_if_ctx.container) {
        snmp_log(LOG_ERR,
                 "could not initialize container for ipIfStatsTable\n");
        return;
    }

    access_multiplexer->object_lookup = _mfd_ipIfStatsTable_object_lookup;
    access_multiplexer->get_values = _mfd_ipIfStatsTable_get_values;

    access_multiplexer->pre_request = _mfd_ipIfStatsTable_pre_request;
    access_multiplexer->post_request = _mfd_ipIfStatsTable_post_request;

    DEBUGMSGTL(("ipIfStatsTable:init_ipIfStatsTable",
                "Registering ipIfStatsTable as a mibs-for-dummies table.\n"));

    handler = netsnmp_baby_steps_access_multiplexer_get(access_multiplexer);
    reginfo = netsnmp_handler_registration_create(
        "ipIfStatsTable", handler, ipIfStatsTable_oid, ipIfStatsTable_oid_size,
        HANDLER_CAN_BABY_STEP | HANDLER_CAN_RONLY);

    if (NULL == reginfo) {
        snmp_log(LOG_ERR, "error registering table ipIfStatsTable\n");
        return;
    }

    reginfo->my_reg_void = &ipIfStatsTable_if_ctx;

    if (access_multiplexer->object_lookup)
        mfd_modes |= BABY_STEP_OBJECT_LOOKUP;
    if (access_multiplexer->pre_request)
        mfd_modes |= BABY_STEP_PRE_REQUEST;
    if (access_multiplexer->post_request)
        mfd_modes |= BABY_STEP_POST_REQUEST;

    handler = netsnmp_baby_steps_handler_get(mfd_modes);
    netsnmp_inject_handler(reginfo, handler);

    handler = netsnmp_get_row_merge_handler(reginfo->rootoid_len + 2);
    netsnmp_inject_handler(reginfo, handler);

    handler = netsnmp_container_table_handler_get(
        tbl_info, ipIfStatsTable_if_ctx.container,
        TABLE_CONTAINER_KEY_NETSNMP_INDEX);
    netsnmp_inject_handler(reginfo, handler);

    if (NULL != ipIfStatsTable_if_ctx.cache) {
        handler = netsnmp_cache_handler_get(ipIfStatsTable_if_ctx.cache);
        netsnmp_inject_handler(reginfo, handler);
    }

    netsnmp_register_table(reginfo, tbl_info);
}

void _ipIfStatsTable_shutdown_interface(ipIfStatsTable_registration *reg_ptr) {
    _ipIfStatsTable_container_shutdown(&ipIfStatsTable_if_ctx);
}

void ipIfStatsTable_valid_columns_set(netsnmp_column_info *vc) {
    ipIfStatsTable_if_ctx.tbl_info.valid_columns = vc;
}

int ipIfStatsTable_index_to_oid(netsnmp_index *oid_idx,
                                ipIfStatsTable_mib_index *mib_idx) {
    int err = SNMP_ERR_NOERROR;
    netsnmp_variable_list var_ipIfStatsIPVersion;
    netsnmp_variable_list var_ipIfStatsIfIndex;

    memset(&var_ipIfStatsIPVersion, 0x00, sizeof(var_ipIfStatsIPVersion));
    var_ipIfStatsIPVersion.type = ASN_INTEGER;
    var_ipIfStatsIPVersion.next_variable = &var_ipIfStatsIfIndex;

    memset(&var_ipIfStatsIfIndex, 0x00, sizeof(var_ipIfStatsIfIndex));
    var_ipIfStatsIfIndex.type = ASN_INTEGER;
    var_ipIfStatsIfIndex.next_variable = NULL;

    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsTable_index_to_oid", "called\n"));

    snmp_set_var_value(&var_ipIfStatsIPVersion, &mib_idx->ipIfStatsIPVersion,
                       sizeof(mib_idx->ipIfStatsIPVersion));
    snmp_set_var_value(&var_ipIfStatsIfIndex, &mib_idx->ipIfStatsIfIndex,
                       sizeof(mib_idx->ipIfStatsIfIndex));
    err = build_oid_noalloc(oid_idx->oids, oid_idx->len, &oid_idx->len, NULL, 0,
                            &var_ipIfStatsIPVersion);
    if (err) {
        snmp_log(LOG_ERR, "error %d converting index to oid\n", err);
    }

    snmp_reset_var_buffers(&var_ipIfStatsIPVersion);
    return err;
}

int ipIfStatsTable_index_from_oid(netsnmp_index *oid_idx,
                                  ipIfStatsTable_mib_index *mib_idx) {
    int err = SNMP_ERR_NOERROR;
    netsnmp_variable_list var_ipIfStatsIPVersion;
    netsnmp_variable_list var_ipIfStatsIfIndex;

    memset(&var_ipIfStatsIPVersion, 0x00, sizeof(var_ipIfStatsIPVersion));
    var_ipIfStatsIPVersion.type = ASN_INTEGER;
    var_ipIfStatsIPVersion.next_variable = &var_ipIfStatsIfIndex;

    memset(&var_ipIfStatsIfIndex, 0x00, sizeof(var_ipIfStatsIfIndex));
    var_ipIfStatsIfIndex.type = ASN_INTEGER;
    var_ipIfStatsIfIndex.next_variable = NULL;

    DEBUGMSGTL(
        ("verbose:ipIfStatsTable:ipIfStatsTable_index_from_oid", "called\n"));

    err =
        parse_oid_indexes(oid_idx->oids, oid_idx->len, &var_ipIfStatsIPVersion);
    if (err == SNMP_ERR_NOERROR) {
        mib_idx->ipIfStatsIPVersion =
            *((long *)var_ipIfStatsIPVersion.val.string);
        mib_idx->ipIfStatsIfIndex = *((long *)var_ipIfStatsIfIndex.val.string);
    }

    snmp_reset_var_buffers(&var_ipIfStatsIPVersion);
    return err;
}

ipIfStatsTable_rowreq_ctx *
ipIfStatsTable_allocate_rowreq_ctx(void *user_init_ctx) {
    ipIfStatsTable_rowreq_ctx *rowreq_ctx =
        SNMP_MALLOC_TYPEDEF(ipIfStatsTable_rowreq_ctx);

    DEBUGMSGTL(("internal:ipIfStatsTable:ipIfStatsTable_allocate_rowreq_ctx",
                "called\n"));

    if (NULL == rowreq_ctx) {
        snmp_log(
            LOG_ERR,
            "Could not allocate memory for a ipIfStatsTable_rowreq_ctx.\n");
        return NULL;
    }

    rowreq_ctx->oid_idx.oids = rowreq_ctx->oid_tmp;
    rowreq_ctx->ipIfStatsTable_data_list = NULL;
    if (!(rowreq_ctx->rowreq_flags & MFD_ROW_DATA_FROM_USER)) {
        if (SNMPERR_SUCCESS !=
            ipIfStatsTable_rowreq_ctx_init(rowreq_ctx, user_init_ctx)) {
            ipIfStatsTable_release_rowreq_ctx(rowreq_ctx);
            rowreq_ctx = NULL;
        }
    }
    return rowreq_ctx;
}

void ipIfStatsTable_release_rowreq_ctx(ipIfStatsTable_rowreq_ctx *rowreq_ctx) {
    DEBUGMSGTL(("internal:ipIfStatsTable:ipIfStatsTable_release_rowreq_ctx",
                "called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    ipIfStatsTable_rowreq_ctx_cleanup(rowreq_ctx);
    if (rowreq_ctx->oid_idx.oids != rowreq_ctx->oid_tmp) {
        free(rowreq_ctx->oid_idx.oids);
    }

    SNMP_FREE(rowreq_ctx);
}

static int _mfd_ipIfStatsTable_pre_request(
    netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *agtreq_info, netsnmp_request_info *requests) {
    int rc;
    DEBUGMSGTL(("internal:ipIfStatsTable:_mfd_ipIfStatsTable_pre_request",
                "called\n"));

    if (1 != netsnmp_row_merge_status_first(reginfo, agtreq_info)) {
        DEBUGMSGTL(
            ("internal:ipIfStatsTable", "skipping additional pre_request\n"));
        return SNMP_ERR_NOERROR;
    }

    rc = ipIfStatsTable_pre_request(ipIfStatsTable_if_ctx.user_ctx);
    if (MFD_SUCCESS != rc) {
        DEBUGMSGTL(("ipIfStatsTable",
                    "error %d from ipIfStatsTable_pre_requests\n", rc));
        netsnmp_request_set_error_all(requests, SNMP_VALIDATE_ERR(rc));
    }

    return SNMP_ERR_NOERROR;
}

static int _mfd_ipIfStatsTable_post_request(
    netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *agtreq_info, netsnmp_request_info *requests) {
    ipIfStatsTable_rowreq_ctx *rowreq_ctx =
        (ipIfStatsTable_rowreq_ctx *)netsnmp_container_table_row_extract(
            requests);
    int rc, packet_rc;
    DEBUGMSGTL(("internal:ipIfStatsTable:_mfd_ipIfStatsTable_post_request",
                "called\n"));

    if (rowreq_ctx && (rowreq_ctx->rowreq_flags & MFD_ROW_DELETED)) {
        ipIfStatsTable_release_rowreq_ctx(rowreq_ctx);
    }

    if (1 != netsnmp_row_merge_status_last(reginfo, agtreq_info)) {
        DEBUGMSGTL(
            ("internal:ipIfStatsTable", "waiting for last post_request\n"));
        return SNMP_ERR_NOERROR;
    }

    packet_rc = netsnmp_check_all_requests_error(agtreq_info->asp, 0);
    rc = ipIfStatsTable_post_request(ipIfStatsTable_if_ctx.user_ctx, packet_rc);
    if (MFD_SUCCESS != rc) {
        DEBUGMSGTL(("ipIfStatsTable",
                    "error %d from ipIfStatsTable_post_request\n", rc));
    }
    return SNMP_ERR_NOERROR;
}

static int _mfd_ipIfStatsTable_object_lookup(
    netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *agtreq_info, netsnmp_request_info *requests) {
    int rc = SNMP_ERR_NOERROR;
    ipIfStatsTable_rowreq_ctx *rowreq_ctx =
        (ipIfStatsTable_rowreq_ctx *)netsnmp_container_table_row_extract(
            requests);

    DEBUGMSGTL(("internal:ipIfStatsTable:_mfd_ipIfStatsTable_object_lookup",
                "called\n"));

    if (NULL == rowreq_ctx) {
        rc = SNMP_ERR_NOCREATION;
    }

    if (MFD_SUCCESS != rc) {
        netsnmp_request_set_error_all(requests, rc);
    } else {
        ipIfStatsTable_row_prep(rowreq_ctx);
    }

    return SNMP_VALIDATE_ERR(rc);
}

NETSNMP_STATIC_INLINE int
_ipIfStatsTable_get_column(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                           netsnmp_variable_list *var, int column) {
    int rc = SNMPERR_SUCCESS;

    DEBUGMSGTL(("internal:ipIfStatsTable:_mfd_ipIfStatsTable_get_column",
                "called for %d\n", column));

    netsnmp_assert(NULL != rowreq_ctx);

    switch (column) {
    case COLUMN_IPIFSTATSINRECEIVES: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsInReceives_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCINRECEIVES: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCInReceives_get(rowreq_ctx,
                                       (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSINOCTETS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsInOctets_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCINOCTETS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCInOctets_get(rowreq_ctx,
                                     (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSINHDRERRORS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsInHdrErrors_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSINNOROUTES: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsInNoRoutes_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSINADDRERRORS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsInAddrErrors_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSINUNKNOWNPROTOS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc =
            ipIfStatsInUnknownProtos_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSINTRUNCATEDPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc =
            ipIfStatsInTruncatedPkts_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSINFORWDATAGRAMS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc =
            ipIfStatsInForwDatagrams_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCINFORWDATAGRAMS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCInForwDatagrams_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSREASMREQDS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsReasmReqds_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSREASMOKS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsReasmOKs_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSREASMFAILS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsReasmFails_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSINDISCARDS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsInDiscards_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSINDELIVERS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsInDelivers_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCINDELIVERS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCInDelivers_get(rowreq_ctx,
                                       (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSOUTREQUESTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsOutRequests_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCOUTREQUESTS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCOutRequests_get(rowreq_ctx,
                                        (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSOUTFORWDATAGRAMS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsOutForwDatagrams_get(rowreq_ctx,
                                           (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCOUTFORWDATAGRAMS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCOutForwDatagrams_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSOUTDISCARDS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsOutDiscards_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSOUTFRAGREQDS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsOutFragReqds_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSOUTFRAGOKS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsOutFragOKs_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSOUTFRAGFAILS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsOutFragFails_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSOUTFRAGCREATES: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsOutFragCreates_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSOUTTRANSMITS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsOutTransmits_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCOUTTRANSMITS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCOutTransmits_get(rowreq_ctx,
                                         (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSOUTOCTETS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsOutOctets_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCOUTOCTETS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCOutOctets_get(rowreq_ctx,
                                      (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSINMCASTPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsInMcastPkts_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCINMCASTPKTS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCInMcastPkts_get(rowreq_ctx,
                                        (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSINMCASTOCTETS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsInMcastOctets_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCINMCASTOCTETS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCInMcastOctets_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSOUTMCASTPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsOutMcastPkts_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCOUTMCASTPKTS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCOutMcastPkts_get(rowreq_ctx,
                                         (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSOUTMCASTOCTETS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsOutMcastOctets_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCOUTMCASTOCTETS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCOutMcastOctets_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSINBCASTPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsInBcastPkts_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCINBCASTPKTS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCInBcastPkts_get(rowreq_ctx,
                                        (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSOUTBCASTPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsOutBcastPkts_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSHCOUTBCASTPKTS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipIfStatsHCOutBcastPkts_get(rowreq_ctx,
                                         (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSDISCONTINUITYTIME: {
        var->type = ASN_TIMETICKS;
        var->val_len = sizeof(long);
        rc =
            ipIfStatsDiscontinuityTime_get(rowreq_ctx, (long *)var->val.string);
    } break;
    case COLUMN_IPIFSTATSREFRESHRATE: {
        var->type = ASN_UNSIGNED;
        var->val_len = sizeof(u_long);
        rc = ipIfStatsRefreshRate_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    default:
        if (IPIFSTATSTABLE_MIN_COL <= column &&
            column <= IPIFSTATSTABLE_MAX_COL) {
            DEBUGMSGTL(
                ("internal:ipIfStatsTable:_mfd_ipIfStatsTable_get_column",
                 "assume column %d is reserved\n", column));
            rc = MFD_SKIP;
        } else {
            snmp_log(LOG_ERR,
                     "unknown column %d in _ipIfStatsTable_get_column\n",
                     column);
        }
        break;
    }

    return rc;
}

int _mfd_ipIfStatsTable_get_values(netsnmp_mib_handler *handler,
                                   netsnmp_handler_registration *reginfo,
                                   netsnmp_agent_request_info *agtreq_info,
                                   netsnmp_request_info *requests) {
    ipIfStatsTable_rowreq_ctx *rowreq_ctx =
        (ipIfStatsTable_rowreq_ctx *)netsnmp_container_table_row_extract(
            requests);
    netsnmp_table_request_info *tri;
    u_char *old_string;
    void (*dataFreeHook)(void *);
    int rc;

    DEBUGMSGTL(
        ("internal:ipIfStatsTable:_mfd_ipIfStatsTable_get_values", "called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    for (; requests; requests = requests->next) {
        old_string = requests->requestvb->val.string;
        dataFreeHook = requests->requestvb->dataFreeHook;
        if (NULL == requests->requestvb->val.string) {
            requests->requestvb->val.string = requests->requestvb->buf;
            requests->requestvb->val_len = sizeof(requests->requestvb->buf);
        } else if (requests->requestvb->buf ==
                   requests->requestvb->val.string) {
            if (requests->requestvb->val_len !=
                sizeof(requests->requestvb->buf)) {
                requests->requestvb->val_len = sizeof(requests->requestvb->buf);
            }
        }

        tri = netsnmp_extract_table_info(requests);
        if (NULL == tri) {
            continue;
        }
        rc = _ipIfStatsTable_get_column(rowreq_ctx, requests->requestvb,
                                        tri->colnum);
        if (rc) {
            if (MFD_SKIP == rc) {
                requests->requestvb->type = SNMP_NOSUCHINSTANCE;
                rc = SNMP_ERR_NOERROR;
            }
        } else if (NULL == requests->requestvb->val.string) {
            snmp_log(LOG_ERR, "NULL varbind data pointer!\n");
            rc = SNMP_ERR_GENERR;
        }
        if (rc) {
            netsnmp_request_set_error(requests, SNMP_VALIDATE_ERR(rc));
        }

        if (old_string && (old_string != requests->requestvb->buf) &&
            (requests->requestvb->val.string != old_string)) {
            if (dataFreeHook) {
                (*dataFreeHook)(old_string);
            } else {
                free(old_string);
            }
        }
    }

    return SNMP_ERR_NOERROR;
}

static void _container_free(netsnmp_container *container);

static int _cache_load(netsnmp_cache *cache, void *vmagic) {
    DEBUGMSGTL(("internal:ipIfStatsTable:_cache_load", "called\n"));

    if ((NULL == cache) || (NULL == cache->magic)) {
        snmp_log(LOG_ERR, "invalid cache for ipIfStatsTable_cache_load\n");
        return -1;
    }

    netsnmp_assert((0 == cache->valid) || (1 == cache->expired));

    return ipIfStatsTable_container_load((netsnmp_container *)cache->magic);
}

static void _cache_free(netsnmp_cache *cache, void *magic) {
    netsnmp_container *container;
    DEBUGMSGTL(("internal:ipIfStatsTable:_cache_free", "called\n"));

    if ((NULL == cache) || (NULL == cache->magic)) {
        snmp_log(LOG_ERR, "invalid cache in ipIfStatsTable_cache_free\n");
        return;
    }

    container = (netsnmp_container *)cache->magic;
    _container_free(container);
}

static void _container_item_free(ipIfStatsTable_rowreq_ctx *rowreq_ctx,
                                 void *context) {
    DEBUGMSGTL(("internal:ipIfStatsTable:_container_item_free", "called\n"));

    if (NULL == rowreq_ctx) {
        return;
    }
    ipIfStatsTable_release_rowreq_ctx(rowreq_ctx);
}

static void _container_free(netsnmp_container *container) {
    DEBUGMSGTL(("internal:ipIfStatsTable:_container_free", "called\n"));

    if (NULL == container) {
        snmp_log(LOG_ERR,
                 "invalid container in ipIfStatsTable_container_free\n");
        return;
    }
    ipIfStatsTable_container_free(container);
    CONTAINER_CLEAR(container,
                    (netsnmp_container_obj_func *)_container_item_free, NULL);
}

void _ipIfStatsTable_container_init(ipIfStatsTable_interface_ctx *if_ctx) {
    DEBUGMSGTL(
        ("internal:ipIfStatsTable:_ipIfStatsTable_container_init", "called\n"));

    if_ctx->cache =
        netsnmp_cache_create(30, _cache_load, _cache_free, ipIfStatsTable_oid,
                             ipIfStatsTable_oid_size);

    if (NULL == if_ctx->cache) {
        snmp_log(LOG_ERR, "error creating cache for ipIfStatsTable\n");
        return;
    }

    if_ctx->cache->flags = NETSNMP_CACHE_DONT_INVALIDATE_ON_SET;
    ipIfStatsTable_container_init(&if_ctx->container, if_ctx->cache);
    if (NULL == if_ctx->container) {
        if_ctx->container =
            netsnmp_container_find("ipIfStatsTable:table_container");
    }
    if (NULL == if_ctx->container) {
        snmp_log(LOG_ERR,
                 "error creating container in ipIfStatsTable_container_init\n");
        return;
    }

    if (NULL != if_ctx->cache) {
        if_ctx->cache->magic = (void *)if_ctx->container;
    }
}

void _ipIfStatsTable_container_shutdown(ipIfStatsTable_interface_ctx *if_ctx) {
    DEBUGMSGTL(("internal:ipIfStatsTable:_ipIfStatsTable_container_shutdown",
                "called\n"));

    ipIfStatsTable_container_shutdown(if_ctx->container);
    _container_free(if_ctx->container);
}

ipIfStatsTable_rowreq_ctx *
ipIfStatsTable_row_find_by_mib_index(ipIfStatsTable_mib_index *mib_idx) {
    ipIfStatsTable_rowreq_ctx *rowreq_ctx;
    oid oid_tmp[MAX_OID_LEN];
    netsnmp_index oid_idx;
    int rc;

    oid_idx.oids = oid_tmp;
    oid_idx.len = sizeof(oid_tmp) / sizeof(oid);

    rc = ipIfStatsTable_index_to_oid(&oid_idx, mib_idx);
    if (MFD_SUCCESS != rc) {
        return NULL;
    }

    rowreq_ctx = (ipIfStatsTable_rowreq_ctx *)CONTAINER_FIND(
        ipIfStatsTable_if_ctx.container, &oid_idx);

    return rowreq_ctx;
}
