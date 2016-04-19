#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/table_container.h>
#include <net-snmp/library/container.h>
#include "ipSystemStatsTable.h"
#include "ipSystemStatsTable_interface.h"

netsnmp_feature_require(baby_steps) netsnmp_feature_require(row_merge)
    netsnmp_feature_require(check_all_requests_error)

        typedef struct ipSystemStatsTable_interface_ctx_s {
    netsnmp_container *container;
    netsnmp_cache *cache;
    ipSystemStatsTable_registration *user_ctx;
    netsnmp_table_registration_info tbl_info;
    netsnmp_baby_steps_access_methods access_multiplexer;
} ipSystemStatsTable_interface_ctx;

static ipSystemStatsTable_interface_ctx ipSystemStatsTable_if_ctx;
static void
_ipSystemStatsTable_container_init(ipSystemStatsTable_interface_ctx *if_ctx);
static void _ipSystemStatsTable_container_shutdown(
    ipSystemStatsTable_interface_ctx *if_ctx);

netsnmp_container *ipSystemStatsTable_container_get(void) {
    return ipSystemStatsTable_if_ctx.container;
}

ipSystemStatsTable_registration *ipSystemStatsTable_registration_get(void) {
    return ipSystemStatsTable_if_ctx.user_ctx;
}

ipSystemStatsTable_registration *
ipSystemStatsTable_registration_set(ipSystemStatsTable_registration *newreg) {
    ipSystemStatsTable_registration *old = ipSystemStatsTable_if_ctx.user_ctx;
    ipSystemStatsTable_if_ctx.user_ctx = newreg;
    return old;
}

int ipSystemStatsTable_container_size(void) {
    return CONTAINER_SIZE(ipSystemStatsTable_if_ctx.container);
}

static Netsnmp_Node_Handler _mfd_ipSystemStatsTable_pre_request;
static Netsnmp_Node_Handler _mfd_ipSystemStatsTable_post_request;
static Netsnmp_Node_Handler _mfd_ipSystemStatsTable_object_lookup;
static Netsnmp_Node_Handler _mfd_ipSystemStatsTable_get_values;

void _ipSystemStatsTable_initialize_interface(
    ipSystemStatsTable_registration *reg_ptr, u_long flags) {
    netsnmp_baby_steps_access_methods *access_multiplexer =
        &ipSystemStatsTable_if_ctx.access_multiplexer;
    netsnmp_table_registration_info *tbl_info =
        &ipSystemStatsTable_if_ctx.tbl_info;
    netsnmp_handler_registration *reginfo;
    netsnmp_mib_handler *handler;
    int mfd_modes = 0;

    DEBUGMSGTL(
        ("internal:ipSystemStatsTable:_ipSystemStatsTable_initialize_interface",
         "called\n"));

    netsnmp_table_helper_add_indexes(tbl_info, ASN_INTEGER, 0);

    tbl_info->min_column = IPSYSTEMSTATSTABLE_MIN_COL;
    tbl_info->max_column = IPSYSTEMSTATSTABLE_MAX_COL;
    ipSystemStatsTable_if_ctx.user_ctx = reg_ptr;
    ipSystemStatsTable_init_data(reg_ptr);
    _ipSystemStatsTable_container_init(&ipSystemStatsTable_if_ctx);
    if (NULL == ipSystemStatsTable_if_ctx.container) {
        snmp_log(LOG_ERR,
                 "could not initialize container for ipSystemStatsTable\n");
        return;
    }

    access_multiplexer->object_lookup = _mfd_ipSystemStatsTable_object_lookup;
    access_multiplexer->get_values = _mfd_ipSystemStatsTable_get_values;

    access_multiplexer->pre_request = _mfd_ipSystemStatsTable_pre_request;
    access_multiplexer->post_request = _mfd_ipSystemStatsTable_post_request;

    DEBUGMSGTL(
        ("ipSystemStatsTable:init_ipSystemStatsTable",
         "Registering ipSystemStatsTable as a mibs-for-dummies table.\n"));

    handler = netsnmp_baby_steps_access_multiplexer_get(access_multiplexer);
    reginfo = netsnmp_handler_registration_create(
        "ipSystemStatsTable", handler, ipSystemStatsTable_oid,
        ipSystemStatsTable_oid_size, HANDLER_CAN_BABY_STEP | HANDLER_CAN_RONLY);

    if (NULL == reginfo) {
        snmp_log(LOG_ERR, "error registering table ipSystemStatsTable\n");
        return;
    }

    reginfo->my_reg_void = &ipSystemStatsTable_if_ctx;

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
        tbl_info, ipSystemStatsTable_if_ctx.container,
        TABLE_CONTAINER_KEY_NETSNMP_INDEX);
    netsnmp_inject_handler(reginfo, handler);

    if (NULL != ipSystemStatsTable_if_ctx.cache) {
        handler = netsnmp_cache_handler_get(ipSystemStatsTable_if_ctx.cache);
        netsnmp_inject_handler(reginfo, handler);
    }

    netsnmp_register_table(reginfo, tbl_info);
}

void _ipSystemStatsTable_shutdown_interface(
    ipSystemStatsTable_registration *reg_ptr) {
    _ipSystemStatsTable_container_shutdown(&ipSystemStatsTable_if_ctx);
}

void ipSystemStatsTable_valid_columns_set(netsnmp_column_info *vc) {
    ipSystemStatsTable_if_ctx.tbl_info.valid_columns = vc;
}

int ipSystemStatsTable_index_to_oid(netsnmp_index *oid_idx,
                                    ipSystemStatsTable_mib_index *mib_idx) {
    int err = SNMP_ERR_NOERROR;
    netsnmp_variable_list var_ipSystemStatsIPVersion;

    memset(&var_ipSystemStatsIPVersion, 0x00,
           sizeof(var_ipSystemStatsIPVersion));
    var_ipSystemStatsIPVersion.type = ASN_INTEGER;
    var_ipSystemStatsIPVersion.next_variable = NULL;

    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsTable_index_to_oid",
                "called\n"));

    snmp_set_var_value(&var_ipSystemStatsIPVersion,
                       &mib_idx->ipSystemStatsIPVersion,
                       sizeof(mib_idx->ipSystemStatsIPVersion));
    err = build_oid_noalloc(oid_idx->oids, oid_idx->len, &oid_idx->len, NULL, 0,
                            &var_ipSystemStatsIPVersion);
    if (err) {
        snmp_log(LOG_ERR, "error %d converting index to oid\n", err);
    }

    snmp_reset_var_buffers(&var_ipSystemStatsIPVersion);
    return err;
}

int ipSystemStatsTable_index_from_oid(netsnmp_index *oid_idx,
                                      ipSystemStatsTable_mib_index *mib_idx) {
    int err = SNMP_ERR_NOERROR;
    netsnmp_variable_list var_ipSystemStatsIPVersion;

    memset(&var_ipSystemStatsIPVersion, 0x00,
           sizeof(var_ipSystemStatsIPVersion));
    var_ipSystemStatsIPVersion.type = ASN_INTEGER;
    var_ipSystemStatsIPVersion.next_variable = NULL;

    DEBUGMSGTL(("verbose:ipSystemStatsTable:ipSystemStatsTable_index_from_oid",
                "called\n"));

    err = parse_oid_indexes(oid_idx->oids, oid_idx->len,
                            &var_ipSystemStatsIPVersion);
    if (err == SNMP_ERR_NOERROR) {
        mib_idx->ipSystemStatsIPVersion =
            *((long *)var_ipSystemStatsIPVersion.val.string);
    }

    snmp_reset_var_buffers(&var_ipSystemStatsIPVersion);
    return err;
}

ipSystemStatsTable_rowreq_ctx *
ipSystemStatsTable_allocate_rowreq_ctx(void *user_init_ctx) {
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx =
        SNMP_MALLOC_TYPEDEF(ipSystemStatsTable_rowreq_ctx);

    DEBUGMSGTL(
        ("internal:ipSystemStatsTable:ipSystemStatsTable_allocate_rowreq_ctx",
         "called\n"));

    if (NULL == rowreq_ctx) {
        snmp_log(
            LOG_ERR,
            "Could not allocate memory for a ipSystemStatsTable_rowreq_ctx.\n");
        return NULL;
    }

    rowreq_ctx->oid_idx.oids = rowreq_ctx->oid_tmp;
    rowreq_ctx->ipSystemStatsTable_data_list = NULL;
    if (!(rowreq_ctx->rowreq_flags & MFD_ROW_DATA_FROM_USER)) {
        if (SNMPERR_SUCCESS !=
            ipSystemStatsTable_rowreq_ctx_init(rowreq_ctx, user_init_ctx)) {
            ipSystemStatsTable_release_rowreq_ctx(rowreq_ctx);
            rowreq_ctx = NULL;
        }
    }
    return rowreq_ctx;
}

void ipSystemStatsTable_release_rowreq_ctx(
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx) {
    DEBUGMSGTL(
        ("internal:ipSystemStatsTable:ipSystemStatsTable_release_rowreq_ctx",
         "called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    ipSystemStatsTable_rowreq_ctx_cleanup(rowreq_ctx);
    if (rowreq_ctx->oid_idx.oids != rowreq_ctx->oid_tmp) {
        free(rowreq_ctx->oid_idx.oids);
    }

    SNMP_FREE(rowreq_ctx);
}

static int _mfd_ipSystemStatsTable_pre_request(
    netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *agtreq_info, netsnmp_request_info *requests) {
    int rc;
    DEBUGMSGTL(
        ("internal:ipSystemStatsTable:_mfd_ipSystemStatsTable_pre_request",
         "called\n"));

    if (1 != netsnmp_row_merge_status_first(reginfo, agtreq_info)) {
        DEBUGMSGTL(("internal:ipSystemStatsTable",
                    "skipping additional pre_request\n"));
        return SNMP_ERR_NOERROR;
    }

    rc = ipSystemStatsTable_pre_request(ipSystemStatsTable_if_ctx.user_ctx);
    if (MFD_SUCCESS != rc) {
        DEBUGMSGTL(("ipSystemStatsTable",
                    "error %d from ipSystemStatsTable_pre_requests\n", rc));
        netsnmp_request_set_error_all(requests, SNMP_VALIDATE_ERR(rc));
    }

    return SNMP_ERR_NOERROR;
}

static int _mfd_ipSystemStatsTable_post_request(
    netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *agtreq_info, netsnmp_request_info *requests) {
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx =
        (ipSystemStatsTable_rowreq_ctx *)netsnmp_container_table_row_extract(
            requests);
    int rc, packet_rc;
    DEBUGMSGTL(
        ("internal:ipSystemStatsTable:_mfd_ipSystemStatsTable_post_request",
         "called\n"));

    if (rowreq_ctx && (rowreq_ctx->rowreq_flags & MFD_ROW_DELETED)) {
        ipSystemStatsTable_release_rowreq_ctx(rowreq_ctx);
    }

    if (1 != netsnmp_row_merge_status_last(reginfo, agtreq_info)) {
        DEBUGMSGTL(
            ("internal:ipSystemStatsTable", "waiting for last post_request\n"));
        return SNMP_ERR_NOERROR;
    }

    packet_rc = netsnmp_check_all_requests_error(agtreq_info->asp, 0);
    rc = ipSystemStatsTable_post_request(ipSystemStatsTable_if_ctx.user_ctx,
                                         packet_rc);
    if (MFD_SUCCESS != rc) {
        DEBUGMSGTL(("ipSystemStatsTable",
                    "error %d from ipSystemStatsTable_post_request\n", rc));
    }
    return SNMP_ERR_NOERROR;
}

static int _mfd_ipSystemStatsTable_object_lookup(
    netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
    netsnmp_agent_request_info *agtreq_info, netsnmp_request_info *requests) {
    int rc = SNMP_ERR_NOERROR;
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx =
        (ipSystemStatsTable_rowreq_ctx *)netsnmp_container_table_row_extract(
            requests);

    DEBUGMSGTL(
        ("internal:ipSystemStatsTable:_mfd_ipSystemStatsTable_object_lookup",
         "called\n"));

    if (NULL == rowreq_ctx) {
        rc = SNMP_ERR_NOCREATION;
    }

    if (MFD_SUCCESS != rc) {
        netsnmp_request_set_error_all(requests, rc);
    } else {
        ipSystemStatsTable_row_prep(rowreq_ctx);
    }

    return SNMP_VALIDATE_ERR(rc);
}

NETSNMP_STATIC_INLINE int
_ipSystemStatsTable_get_column(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                               netsnmp_variable_list *var, int column) {
    int rc = SNMPERR_SUCCESS;

    DEBUGMSGTL(
        ("internal:ipSystemStatsTable:_mfd_ipSystemStatsTable_get_column",
         "called for %d\n", column));

    netsnmp_assert(NULL != rowreq_ctx);

    switch (column) {
    case COLUMN_IPSYSTEMSTATSINRECEIVES: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsInReceives_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCINRECEIVES: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCInReceives_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSINOCTETS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsInOctets_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCINOCTETS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCInOctets_get(rowreq_ctx,
                                         (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSINHDRERRORS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc =
            ipSystemStatsInHdrErrors_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSINNOROUTES: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsInNoRoutes_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSINADDRERRORS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsInAddrErrors_get(rowreq_ctx,
                                           (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSINUNKNOWNPROTOS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsInUnknownProtos_get(rowreq_ctx,
                                              (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSINTRUNCATEDPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsInTruncatedPkts_get(rowreq_ctx,
                                              (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSINFORWDATAGRAMS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsInForwDatagrams_get(rowreq_ctx,
                                              (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCINFORWDATAGRAMS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCInForwDatagrams_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSREASMREQDS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsReasmReqds_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSREASMOKS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsReasmOKs_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSREASMFAILS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsReasmFails_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSINDISCARDS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsInDiscards_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSINDELIVERS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsInDelivers_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCINDELIVERS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCInDelivers_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTREQUESTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc =
            ipSystemStatsOutRequests_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCOUTREQUESTS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCOutRequests_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTNOROUTES: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc =
            ipSystemStatsOutNoRoutes_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTFORWDATAGRAMS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsOutForwDatagrams_get(rowreq_ctx,
                                               (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCOUTFORWDATAGRAMS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCOutForwDatagrams_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTDISCARDS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc =
            ipSystemStatsOutDiscards_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTFRAGREQDS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsOutFragReqds_get(rowreq_ctx,
                                           (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTFRAGOKS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsOutFragOKs_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTFRAGFAILS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsOutFragFails_get(rowreq_ctx,
                                           (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTFRAGCREATES: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsOutFragCreates_get(rowreq_ctx,
                                             (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTTRANSMITS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsOutTransmits_get(rowreq_ctx,
                                           (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCOUTTRANSMITS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCOutTransmits_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTOCTETS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsOutOctets_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCOUTOCTETS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCOutOctets_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSINMCASTPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc =
            ipSystemStatsInMcastPkts_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCINMCASTPKTS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCInMcastPkts_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSINMCASTOCTETS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsInMcastOctets_get(rowreq_ctx,
                                            (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCINMCASTOCTETS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCInMcastOctets_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTMCASTPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsOutMcastPkts_get(rowreq_ctx,
                                           (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCOUTMCASTPKTS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCOutMcastPkts_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTMCASTOCTETS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsOutMcastOctets_get(rowreq_ctx,
                                             (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCOUTMCASTOCTETS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCOutMcastOctets_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSINBCASTPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc =
            ipSystemStatsInBcastPkts_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCINBCASTPKTS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCInBcastPkts_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSOUTBCASTPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsOutBcastPkts_get(rowreq_ctx,
                                           (u_long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSHCOUTBCASTPKTS: {
        var->type = ASN_COUNTER64;
        var->val_len = sizeof(unsigned long long);
        rc = ipSystemStatsHCOutBcastPkts_get(
            rowreq_ctx, (unsigned long long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSDISCONTINUITYTIME: {
        var->type = ASN_TIMETICKS;
        var->val_len = sizeof(long);
        rc = ipSystemStatsDiscontinuityTime_get(rowreq_ctx,
                                                (long *)var->val.string);
    } break;
    case COLUMN_IPSYSTEMSTATSREFRESHRATE: {
        var->type = ASN_UNSIGNED;
        var->val_len = sizeof(u_long);
        rc = ipSystemStatsRefreshRate_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    default:
        if (IPSYSTEMSTATSTABLE_MIN_COL <= column &&
            column <= IPSYSTEMSTATSTABLE_MAX_COL) {
            DEBUGMSGTL(("internal:ipSystemStatsTable:_mfd_ipSystemStatsTable_"
                        "get_column",
                        "assume column %d is reserved\n", column));
            rc = MFD_SKIP;
        } else {
            snmp_log(LOG_ERR,
                     "unknown column %d in _ipSystemStatsTable_get_column\n",
                     column);
        }
        break;
    }

    return rc;
}

int _mfd_ipSystemStatsTable_get_values(netsnmp_mib_handler *handler,
                                       netsnmp_handler_registration *reginfo,
                                       netsnmp_agent_request_info *agtreq_info,
                                       netsnmp_request_info *requests) {
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx =
        (ipSystemStatsTable_rowreq_ctx *)netsnmp_container_table_row_extract(
            requests);
    netsnmp_table_request_info *tri;
    u_char *old_string;
    void (*dataFreeHook)(void *);
    int rc;

    DEBUGMSGTL(
        ("internal:ipSystemStatsTable:_mfd_ipSystemStatsTable_get_values",
         "called\n"));

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
        rc = _ipSystemStatsTable_get_column(rowreq_ctx, requests->requestvb,
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
    DEBUGMSGTL(("internal:ipSystemStatsTable:_cache_load", "called\n"));

    if ((NULL == cache) || (NULL == cache->magic)) {
        snmp_log(LOG_ERR, "invalid cache for ipSystemStatsTable_cache_load\n");
        return -1;
    }

    netsnmp_assert((0 == cache->valid) || (1 == cache->expired));

    return ipSystemStatsTable_container_load((netsnmp_container *)cache->magic);
}

static void _cache_free(netsnmp_cache *cache, void *magic) {
    netsnmp_container *container;
    DEBUGMSGTL(("internal:ipSystemStatsTable:_cache_free", "called\n"));

    if ((NULL == cache) || (NULL == cache->magic)) {
        snmp_log(LOG_ERR, "invalid cache in ipSystemStatsTable_cache_free\n");
        return;
    }

    container = (netsnmp_container *)cache->magic;
    _container_free(container);
}

static void _container_item_free(ipSystemStatsTable_rowreq_ctx *rowreq_ctx,
                                 void *context) {
    DEBUGMSGTL(
        ("internal:ipSystemStatsTable:_container_item_free", "called\n"));

    if (NULL == rowreq_ctx) {
        return;
    }
    ipSystemStatsTable_release_rowreq_ctx(rowreq_ctx);
}

static void _container_free(netsnmp_container *container) {
    DEBUGMSGTL(("internal:ipSystemStatsTable:_container_free", "called\n"));

    if (NULL == container) {
        snmp_log(LOG_ERR,
                 "invalid container in ipSystemStatsTable_container_free\n");
        return;
    }
    ipSystemStatsTable_container_free(container);
    CONTAINER_CLEAR(container,
                    (netsnmp_container_obj_func *)_container_item_free, NULL);
}

void _ipSystemStatsTable_container_init(
    ipSystemStatsTable_interface_ctx *if_ctx) {
    DEBUGMSGTL(
        ("internal:ipSystemStatsTable:_ipSystemStatsTable_container_init",
         "called\n"));

    if_ctx->cache = netsnmp_cache_create(30, _cache_load, _cache_free,
                                         ipSystemStatsTable_oid,
                                         ipSystemStatsTable_oid_size);

    if (NULL == if_ctx->cache) {
        snmp_log(LOG_ERR, "error creating cache for ipSystemStatsTable\n");
        return;
    }

    if_ctx->cache->flags = NETSNMP_CACHE_DONT_INVALIDATE_ON_SET;
    ipSystemStatsTable_container_init(&if_ctx->container, if_ctx->cache);
    if (NULL == if_ctx->container) {
        if_ctx->container =
            netsnmp_container_find("ipSystemStatsTable:table_container");
    }
    if (NULL == if_ctx->container) {
        snmp_log(
            LOG_ERR,
            "error creating container in ipSystemStatsTable_container_init\n");
        return;
    }

    if (NULL != if_ctx->cache) {
        if_ctx->cache->magic = (void *)if_ctx->container;
    }
}

void _ipSystemStatsTable_container_shutdown(
    ipSystemStatsTable_interface_ctx *if_ctx) {
    DEBUGMSGTL(
        ("internal:ipSystemStatsTable:_ipSystemStatsTable_container_shutdown",
         "called\n"));

    ipSystemStatsTable_container_shutdown(if_ctx->container);
    _container_free(if_ctx->container);
}

ipSystemStatsTable_rowreq_ctx *ipSystemStatsTable_row_find_by_mib_index(
    ipSystemStatsTable_mib_index *mib_idx) {
    ipSystemStatsTable_rowreq_ctx *rowreq_ctx;
    oid oid_tmp[MAX_OID_LEN];
    netsnmp_index oid_idx;
    int rc;

    oid_idx.oids = oid_tmp;
    oid_idx.len = sizeof(oid_tmp) / sizeof(oid);

    rc = ipSystemStatsTable_index_to_oid(&oid_idx, mib_idx);
    if (MFD_SUCCESS != rc) {
        return NULL;
    }

    rowreq_ctx = (ipSystemStatsTable_rowreq_ctx *)CONTAINER_FIND(
        ipSystemStatsTable_if_ctx.container, &oid_idx);

    return rowreq_ctx;
}
