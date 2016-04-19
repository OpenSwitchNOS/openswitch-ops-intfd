#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/table_container.h>
#include <net-snmp/library/container.h>
#include "ifTable.h"
#include "ifTable_interface.h"

netsnmp_feature_require(baby_steps) netsnmp_feature_require(row_merge)
    netsnmp_feature_require(check_all_requests_error)

        typedef struct ifTable_interface_ctx_s {
    netsnmp_container *container;
    netsnmp_cache *cache;
    ifTable_registration *user_ctx;
    netsnmp_table_registration_info tbl_info;
    netsnmp_baby_steps_access_methods access_multiplexer;
} ifTable_interface_ctx;

static ifTable_interface_ctx ifTable_if_ctx;
static void _ifTable_container_init(ifTable_interface_ctx *if_ctx);
static void _ifTable_container_shutdown(ifTable_interface_ctx *if_ctx);

netsnmp_container *ifTable_container_get(void) {
    return ifTable_if_ctx.container;
}

ifTable_registration *ifTable_registration_get(void) {
    return ifTable_if_ctx.user_ctx;
}

ifTable_registration *ifTable_registration_set(ifTable_registration *newreg) {
    ifTable_registration *old = ifTable_if_ctx.user_ctx;
    ifTable_if_ctx.user_ctx = newreg;
    return old;
}

int ifTable_container_size(void) {
    return CONTAINER_SIZE(ifTable_if_ctx.container);
}

static Netsnmp_Node_Handler _mfd_ifTable_pre_request;
static Netsnmp_Node_Handler _mfd_ifTable_post_request;
static Netsnmp_Node_Handler _mfd_ifTable_object_lookup;
static Netsnmp_Node_Handler _mfd_ifTable_get_values;

void _ifTable_initialize_interface(ifTable_registration *reg_ptr,
                                   u_long flags) {
    netsnmp_baby_steps_access_methods *access_multiplexer =
        &ifTable_if_ctx.access_multiplexer;
    netsnmp_table_registration_info *tbl_info = &ifTable_if_ctx.tbl_info;
    netsnmp_handler_registration *reginfo;
    netsnmp_mib_handler *handler;
    int mfd_modes = 0;

    DEBUGMSGTL(("internal:ifTable:_ifTable_initialize_interface", "called\n"));

    netsnmp_table_helper_add_indexes(tbl_info, ASN_INTEGER, 0);

    tbl_info->min_column = IFTABLE_MIN_COL;
    tbl_info->max_column = IFTABLE_MAX_COL;
    ifTable_if_ctx.user_ctx = reg_ptr;
    ifTable_init_data(reg_ptr);
    _ifTable_container_init(&ifTable_if_ctx);
    if (NULL == ifTable_if_ctx.container) {
        snmp_log(LOG_ERR, "could not initialize container for ifTable\n");
        return;
    }

    access_multiplexer->object_lookup = _mfd_ifTable_object_lookup;
    access_multiplexer->get_values = _mfd_ifTable_get_values;

    access_multiplexer->pre_request = _mfd_ifTable_pre_request;
    access_multiplexer->post_request = _mfd_ifTable_post_request;

    DEBUGMSGTL(("ifTable:init_ifTable",
                "Registering ifTable as a mibs-for-dummies table.\n"));

    handler = netsnmp_baby_steps_access_multiplexer_get(access_multiplexer);
    reginfo = netsnmp_handler_registration_create(
        "ifTable", handler, ifTable_oid, ifTable_oid_size,
        HANDLER_CAN_BABY_STEP | HANDLER_CAN_RONLY);

    if (NULL == reginfo) {
        snmp_log(LOG_ERR, "error registering table ifTable\n");
        return;
    }

    reginfo->my_reg_void = &ifTable_if_ctx;

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
        tbl_info, ifTable_if_ctx.container, TABLE_CONTAINER_KEY_NETSNMP_INDEX);
    netsnmp_inject_handler(reginfo, handler);

    if (NULL != ifTable_if_ctx.cache) {
        handler = netsnmp_cache_handler_get(ifTable_if_ctx.cache);
        netsnmp_inject_handler(reginfo, handler);
    }

    netsnmp_register_table(reginfo, tbl_info);
}

void _ifTable_shutdown_interface(ifTable_registration *reg_ptr) {
    _ifTable_container_shutdown(&ifTable_if_ctx);
}

void ifTable_valid_columns_set(netsnmp_column_info *vc) {
    ifTable_if_ctx.tbl_info.valid_columns = vc;
}

int ifTable_index_to_oid(netsnmp_index *oid_idx, ifTable_mib_index *mib_idx) {
    int err = SNMP_ERR_NOERROR;
    netsnmp_variable_list var_ifIndex;

    memset(&var_ifIndex, 0x00, sizeof(var_ifIndex));
    var_ifIndex.type = ASN_INTEGER;
    var_ifIndex.next_variable = NULL;

    DEBUGMSGTL(("verbose:ifTable:ifTable_index_to_oid", "called\n"));

    snmp_set_var_value(&var_ifIndex, &mib_idx->ifIndex,
                       sizeof(mib_idx->ifIndex));
    err = build_oid_noalloc(oid_idx->oids, oid_idx->len, &oid_idx->len, NULL, 0,
                            &var_ifIndex);
    if (err) {
        snmp_log(LOG_ERR, "error %d converting index to oid\n", err);
    }

    snmp_reset_var_buffers(&var_ifIndex);
    return err;
}

int ifTable_index_from_oid(netsnmp_index *oid_idx, ifTable_mib_index *mib_idx) {
    int err = SNMP_ERR_NOERROR;
    netsnmp_variable_list var_ifIndex;

    memset(&var_ifIndex, 0x00, sizeof(var_ifIndex));
    var_ifIndex.type = ASN_INTEGER;
    var_ifIndex.next_variable = NULL;

    DEBUGMSGTL(("verbose:ifTable:ifTable_index_from_oid", "called\n"));

    err = parse_oid_indexes(oid_idx->oids, oid_idx->len, &var_ifIndex);
    if (err == SNMP_ERR_NOERROR) {
        mib_idx->ifIndex = *((long *)var_ifIndex.val.string);
    }

    snmp_reset_var_buffers(&var_ifIndex);
    return err;
}

ifTable_rowreq_ctx *ifTable_allocate_rowreq_ctx(void *user_init_ctx) {
    ifTable_rowreq_ctx *rowreq_ctx = SNMP_MALLOC_TYPEDEF(ifTable_rowreq_ctx);

    DEBUGMSGTL(("internal:ifTable:ifTable_allocate_rowreq_ctx", "called\n"));

    if (NULL == rowreq_ctx) {
        snmp_log(LOG_ERR,
                 "Could not allocate memory for a ifTable_rowreq_ctx.\n");
        return NULL;
    }

    rowreq_ctx->oid_idx.oids = rowreq_ctx->oid_tmp;
    rowreq_ctx->ifTable_data_list = NULL;
    if (!(rowreq_ctx->rowreq_flags & MFD_ROW_DATA_FROM_USER)) {
        if (SNMPERR_SUCCESS !=
            ifTable_rowreq_ctx_init(rowreq_ctx, user_init_ctx)) {
            ifTable_release_rowreq_ctx(rowreq_ctx);
            rowreq_ctx = NULL;
        }
    }
    return rowreq_ctx;
}

void ifTable_release_rowreq_ctx(ifTable_rowreq_ctx *rowreq_ctx) {
    DEBUGMSGTL(("internal:ifTable:ifTable_release_rowreq_ctx", "called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    ifTable_rowreq_ctx_cleanup(rowreq_ctx);
    if (rowreq_ctx->oid_idx.oids != rowreq_ctx->oid_tmp) {
        free(rowreq_ctx->oid_idx.oids);
    }

    SNMP_FREE(rowreq_ctx);
}

static int _mfd_ifTable_pre_request(netsnmp_mib_handler *handler,
                                    netsnmp_handler_registration *reginfo,
                                    netsnmp_agent_request_info *agtreq_info,
                                    netsnmp_request_info *requests) {
    int rc;
    DEBUGMSGTL(("internal:ifTable:_mfd_ifTable_pre_request", "called\n"));

    if (1 != netsnmp_row_merge_status_first(reginfo, agtreq_info)) {
        DEBUGMSGTL(("internal:ifTable", "skipping additional pre_request\n"));
        return SNMP_ERR_NOERROR;
    }

    rc = ifTable_pre_request(ifTable_if_ctx.user_ctx);
    if (MFD_SUCCESS != rc) {
        DEBUGMSGTL(("ifTable", "error %d from ifTable_pre_requests\n", rc));
        netsnmp_request_set_error_all(requests, SNMP_VALIDATE_ERR(rc));
    }

    return SNMP_ERR_NOERROR;
}

static int _mfd_ifTable_post_request(netsnmp_mib_handler *handler,
                                     netsnmp_handler_registration *reginfo,
                                     netsnmp_agent_request_info *agtreq_info,
                                     netsnmp_request_info *requests) {
    ifTable_rowreq_ctx *rowreq_ctx =
        (ifTable_rowreq_ctx *)netsnmp_container_table_row_extract(requests);
    int rc, packet_rc;
    DEBUGMSGTL(("internal:ifTable:_mfd_ifTable_post_request", "called\n"));

    if (rowreq_ctx && (rowreq_ctx->rowreq_flags & MFD_ROW_DELETED)) {
        ifTable_release_rowreq_ctx(rowreq_ctx);
    }

    if (1 != netsnmp_row_merge_status_last(reginfo, agtreq_info)) {
        DEBUGMSGTL(("internal:ifTable", "waiting for last post_request\n"));
        return SNMP_ERR_NOERROR;
    }

    packet_rc = netsnmp_check_all_requests_error(agtreq_info->asp, 0);
    rc = ifTable_post_request(ifTable_if_ctx.user_ctx, packet_rc);
    if (MFD_SUCCESS != rc) {
        DEBUGMSGTL(("ifTable", "error %d from ifTable_post_request\n", rc));
    }
    return SNMP_ERR_NOERROR;
}

static int _mfd_ifTable_object_lookup(netsnmp_mib_handler *handler,
                                      netsnmp_handler_registration *reginfo,
                                      netsnmp_agent_request_info *agtreq_info,
                                      netsnmp_request_info *requests) {
    int rc = SNMP_ERR_NOERROR;
    ifTable_rowreq_ctx *rowreq_ctx =
        (ifTable_rowreq_ctx *)netsnmp_container_table_row_extract(requests);

    DEBUGMSGTL(("internal:ifTable:_mfd_ifTable_object_lookup", "called\n"));

    if (NULL == rowreq_ctx) {
        rc = SNMP_ERR_NOCREATION;
    }

    if (MFD_SUCCESS != rc) {
        netsnmp_request_set_error_all(requests, rc);
    } else {
        ifTable_row_prep(rowreq_ctx);
    }

    return SNMP_VALIDATE_ERR(rc);
}

NETSNMP_STATIC_INLINE int _ifTable_get_column(ifTable_rowreq_ctx *rowreq_ctx,
                                              netsnmp_variable_list *var,
                                              int column) {
    int rc = SNMPERR_SUCCESS;

    DEBUGMSGTL(("internal:ifTable:_mfd_ifTable_get_column", "called for %d\n",
                column));

    netsnmp_assert(NULL != rowreq_ctx);

    switch (column) {
    case COLUMN_IFDESCR: {
        var->type = ASN_OCTET_STR;
        rc = ifDescr_get(rowreq_ctx, (char **)&var->val.string, &var->val_len);
    } break;
    case COLUMN_IFTYPE: {
        var->type = ASN_INTEGER;
        var->val_len = sizeof(long);
        rc = ifType_get(rowreq_ctx, (long *)var->val.string);
    } break;
    case COLUMN_IFMTU: {
        var->type = ASN_INTEGER;
        var->val_len = sizeof(long);
        rc = ifMtu_get(rowreq_ctx, (long *)var->val.string);
    } break;
    case COLUMN_IFSPEED: {
        var->type = ASN_GAUGE;
        var->val_len = sizeof(u_long);
        rc = ifSpeed_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFPHYSADDRESS: {
        var->type = ASN_OCTET_STR;
        rc = ifPhysAddress_get(rowreq_ctx, (char **)&var->val.string,
                               &var->val_len);
    } break;
    case COLUMN_IFADMINSTATUS: {
        var->type = ASN_INTEGER;
        var->val_len = sizeof(long);
        rc = ifAdminStatus_get(rowreq_ctx, (long *)var->val.string);
    } break;
    case COLUMN_IFOPERSTATUS: {
        var->type = ASN_INTEGER;
        var->val_len = sizeof(long);
        rc = ifOperStatus_get(rowreq_ctx, (long *)var->val.string);
    } break;
    case COLUMN_IFLASTCHANGE: {
        var->type = ASN_TIMETICKS;
        var->val_len = sizeof(long);
        rc = ifLastChange_get(rowreq_ctx, (long *)var->val.string);
    } break;
    case COLUMN_IFINOCTETS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ifInOctets_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFINUCASTPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ifInUcastPkts_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFINNUCASTPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ifInNUcastPkts_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFINDISCARDS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ifInDiscards_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFINERRORS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ifInErrors_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFINUNKNOWNPROTOS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ifInUnknownProtos_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFOUTOCTETS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ifOutOctets_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFOUTUCASTPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ifOutUcastPkts_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFOUTNUCASTPKTS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ifOutNUcastPkts_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFOUTDISCARDS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ifOutDiscards_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFOUTERRORS: {
        var->type = ASN_COUNTER;
        var->val_len = sizeof(u_long);
        rc = ifOutErrors_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFOUTQLEN: {
        var->type = ASN_GAUGE;
        var->val_len = sizeof(u_long);
        rc = ifOutQLen_get(rowreq_ctx, (u_long *)var->val.string);
    } break;
    case COLUMN_IFSPECIFIC: {
        var->type = ASN_OBJECT_ID;
        rc =
            ifSpecific_get(rowreq_ctx, (oid **)&var->val.string, &var->val_len);
    } break;
    default:
        if (IFTABLE_MIN_COL <= column && column <= IFTABLE_MAX_COL) {
            DEBUGMSGTL(("internal:ifTable:_mfd_ifTable_get_column",
                        "assume column %d is reserved\n", column));
            rc = MFD_SKIP;
        } else {
            snmp_log(LOG_ERR, "unknown column %d in _ifTable_get_column\n",
                     column);
        }
        break;
    }

    return rc;
}

int _mfd_ifTable_get_values(netsnmp_mib_handler *handler,
                            netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *agtreq_info,
                            netsnmp_request_info *requests) {
    ifTable_rowreq_ctx *rowreq_ctx =
        (ifTable_rowreq_ctx *)netsnmp_container_table_row_extract(requests);
    netsnmp_table_request_info *tri;
    u_char *old_string;
    void (*dataFreeHook)(void *);
    int rc;

    DEBUGMSGTL(("internal:ifTable:_mfd_ifTable_get_values", "called\n"));

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
        rc = _ifTable_get_column(rowreq_ctx, requests->requestvb, tri->colnum);
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
    DEBUGMSGTL(("internal:ifTable:_cache_load", "called\n"));

    if ((NULL == cache) || (NULL == cache->magic)) {
        snmp_log(LOG_ERR, "invalid cache for ifTable_cache_load\n");
        return -1;
    }

    netsnmp_assert((0 == cache->valid) || (1 == cache->expired));

    return ifTable_container_load((netsnmp_container *)cache->magic);
}

static void _cache_free(netsnmp_cache *cache, void *magic) {
    netsnmp_container *container;
    DEBUGMSGTL(("internal:ifTable:_cache_free", "called\n"));

    if ((NULL == cache) || (NULL == cache->magic)) {
        snmp_log(LOG_ERR, "invalid cache in ifTable_cache_free\n");
        return;
    }

    container = (netsnmp_container *)cache->magic;
    _container_free(container);
}

static void _container_item_free(ifTable_rowreq_ctx *rowreq_ctx,
                                 void *context) {
    DEBUGMSGTL(("internal:ifTable:_container_item_free", "called\n"));

    if (NULL == rowreq_ctx) {
        return;
    }
    ifTable_release_rowreq_ctx(rowreq_ctx);
}

static void _container_free(netsnmp_container *container) {
    DEBUGMSGTL(("internal:ifTable:_container_free", "called\n"));

    if (NULL == container) {
        snmp_log(LOG_ERR, "invalid container in ifTable_container_free\n");
        return;
    }
    ifTable_container_free(container);
    CONTAINER_CLEAR(container,
                    (netsnmp_container_obj_func *)_container_item_free, NULL);
}

void _ifTable_container_init(ifTable_interface_ctx *if_ctx) {
    DEBUGMSGTL(("internal:ifTable:_ifTable_container_init", "called\n"));

    if_ctx->cache = netsnmp_cache_create(30, _cache_load, _cache_free,
                                         ifTable_oid, ifTable_oid_size);

    if (NULL == if_ctx->cache) {
        snmp_log(LOG_ERR, "error creating cache for ifTable\n");
        return;
    }

    if_ctx->cache->flags = NETSNMP_CACHE_DONT_INVALIDATE_ON_SET;
    ifTable_container_init(&if_ctx->container, if_ctx->cache);
    if (NULL == if_ctx->container) {
        if_ctx->container = netsnmp_container_find("ifTable:table_container");
    }
    if (NULL == if_ctx->container) {
        snmp_log(LOG_ERR,
                 "error creating container in ifTable_container_init\n");
        return;
    }

    if (NULL != if_ctx->cache) {
        if_ctx->cache->magic = (void *)if_ctx->container;
    }
}

void _ifTable_container_shutdown(ifTable_interface_ctx *if_ctx) {
    DEBUGMSGTL(("internal:ifTable:_ifTable_container_shutdown", "called\n"));

    ifTable_container_shutdown(if_ctx->container);
    _container_free(if_ctx->container);
}

ifTable_rowreq_ctx *ifTable_row_find_by_mib_index(ifTable_mib_index *mib_idx) {
    ifTable_rowreq_ctx *rowreq_ctx;
    oid oid_tmp[MAX_OID_LEN];
    netsnmp_index oid_idx;
    int rc;

    oid_idx.oids = oid_tmp;
    oid_idx.len = sizeof(oid_tmp) / sizeof(oid);

    rc = ifTable_index_to_oid(&oid_idx, mib_idx);
    if (MFD_SUCCESS != rc) {
        return NULL;
    }

    rowreq_ctx = (ifTable_rowreq_ctx *)CONTAINER_FIND(ifTable_if_ctx.container,
                                                      &oid_idx);

    return rowreq_ctx;
}
