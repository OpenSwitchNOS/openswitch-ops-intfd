/*
 * (c) Copyright 2015 Hewlett Packard Enterprise Development LP
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
/************************************************************************//**
 * @ingroup intfd
 *
 * @file
 * Source for intfd OVSDB access interface.
 *
 ***************************************************************************/

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <config.h>
#include <command-line.h>
#include <compiler.h>
#include <daemon.h>
#include <dirs.h>
#include <dynamic-string.h>
#include <fatal-signal.h>
#include <ovsdb-idl.h>
#include <poll-loop.h>
#include <unixctl.h>
#include <util.h>
#include <openvswitch/vconn.h>
#include <openvswitch/vlog.h>
#include <vswitch-idl.h>
#include <openswitch-idl.h>
#include <hash.h>
#include <shash.h>

#include "intfd.h"
#include "intfd_utils.h"

#include "eventlog.h"

VLOG_DEFINE_THIS_MODULE(intfd_ovsdb_if);

/** @ingroup intfd
 * @{ */
static struct ovsdb_idl *idl;

static unsigned int idl_seqno;

static bool system_configured = false;

/* Mapping of all the interfaces. */
static struct shash all_interfaces = SHASH_INITIALIZER(&all_interfaces);

/* Mapping of all the ports. */
static struct shash all_ports = SHASH_INITIALIZER(&all_ports);

struct intf_hw_info {
    bool is_pluggable;
    enum ovsrec_interface_hw_intf_connector_e      connector;
    uint32_t    speeds[INTFD_MAX_SPEEDS_ALLOWED];
    int32_t     n_speeds;
    uint32_t    max_speed;
};

struct intf_user_cfg {
    enum ovsrec_interface_user_config_admin_e      admin_state;
    enum ovsrec_interface_user_config_autoneg_e    autoneg;
    enum ovsrec_interface_user_config_pause_e      pause;
    enum ovsrec_interface_user_config_duplex_e     duplex;
    enum ovsrec_interface_user_config_lane_split_e lane_split;

    uint32_t   speeds[INTFD_MAX_SPEEDS_ALLOWED];
    int32_t    n_speeds;
    int32_t    mtu;
};

struct intf_oper_state {
    bool        enabled;
    enum ovsrec_interface_error_e reason;
    enum ovsrec_interface_error_e autoneg_reason;

    enum ovsrec_interface_hw_intf_config_duplex_e   duplex;
    enum ovsrec_interface_hw_intf_config_pause_e    pause;
    int32_t     autoneg_capability;
    int32_t     autoneg_state;
    int32_t     mtu;
    uint32_t    speeds[INTFD_MAX_SPEEDS_ALLOWED];
    int32_t     n_speeds;
};

struct intf_pm_info {
    uint64_t    op_connector_flags;
    enum ovsrec_interface_pm_info_connector_e           connector;
    enum ovsrec_interface_pm_info_connector_status_e    connector_status;

    enum ovsrec_interface_hw_intf_config_interface_type_e   intf_type;
};

struct iface {
    char                        *name;
    struct intf_hw_info         hw_info;
    enum ovsrec_port_config_admin_e  port_admin;
    char                        *type;
    struct intf_user_cfg        user_cfg;
    struct intf_oper_state      op_state;
    struct intf_pm_info         pm_info;
    struct iface                *split_parent;
    struct iface                **split_children;
    int                         n_split_children;
};

struct port_info {
    char                      *name;
    size_t                    n_interfaces;
    struct ovsrec_interface   **interface;
};

char *interface_pm_info_connector_strings[] = {
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP28_CLR4,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP28_CR4,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP28_PSM4,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP28_CWDM4,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP28_LR4,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP28_SR4,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP28_CR,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP28_LR,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP28_SR,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP_CR4,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP_LR4,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP_SR4,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_CX,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_DAC,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_FC,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_LR,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_LRM,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_LX,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_RJ45,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_SR,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_SX,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_ABSENT,
    OVSREC_INTERFACE_PM_INFO_CONNECTOR_UNKNOWN
};

/* The strings that indicate the autoneg configuration on an interface. */
char *iface_config_autoneg_strings[] = {
    INTERFACE_USER_CONFIG_MAP_AUTONEG_OFF,
    INTERFACE_USER_CONFIG_MAP_AUTONEG_ON,
    INTERFACE_USER_CONFIG_MAP_AUTONEG_DEFAULT
};

typedef struct subsystem {
    char         *name;
    int32_t      mtu;
} subsystem_t;

/* FIXME: Need to modify to handle multiple subsystems */
/* Hardcoding this for now as the base subsytem */
subsystem_t                     base_subsys = {0};

static void del_old_interface(struct shash_node *sh_node);

void set_interface_config(const struct ovsrec_interface *ifrow, struct iface *intf);
int remove_interface_from_port(const struct ovsrec_port *port_row);

void
intfd_debug_dump(struct ds *ds, int argc, const char *argv[])
{
    struct shash_node *sh_node;
    bool list_all_intf = true;
    const char *interface_name;
    int i;

    if (argc > 1) {
        list_all_intf = false;
        interface_name = argv[1];
    }

    if (list_all_intf) {
        ds_put_cstr(ds, "================ Interfaces ================\n");
    } else {
        ds_put_format(ds, "================ Interface %s ================\n",
            interface_name);
    }

    SHASH_FOR_EACH(sh_node, &all_interfaces) {
        struct iface *intf = sh_node->data;
        if (list_all_intf
            || (!strcmp(interface_name, sh_node->name))) {

            ds_put_format(ds, "Interface %s:\n", intf->name);
            ds_put_format(ds, "    admin              : %d\n",
                          intf->user_cfg.admin_state);
            ds_put_format(ds, "    hw_enable          : %d\n",
                          intf->op_state.enabled);
            ds_put_format(ds, "    op_state_reason    : %s\n",
                          intfd_get_error_str(intf->op_state.reason));
            ds_put_format(ds, "    cfg_autoneg        : %s\n",
                          iface_config_autoneg_strings[intf->user_cfg.autoneg]);
            ds_put_format(ds, "    op_autoneg_state   : %d\n",
                          intf->op_state.autoneg_state);
            ds_put_format(ds, "    cfg_speeds         : ");
            if (intf->user_cfg.n_speeds > 0) {
                ds_put_format(ds, "%d", intf->user_cfg.speeds[0]);
                for (i = 1; i < intf->user_cfg.n_speeds; i++) {
                    ds_put_format(ds, ", %d", intf->user_cfg.speeds[i]);
                }
            } else {
                ds_put_format(ds, "unset");
            }
            ds_put_format(ds, "\n");

            ds_put_format(ds, "    op_speeds          : ");
            if (intf->op_state.n_speeds > 0) {
                ds_put_format(ds, "%d", intf->op_state.speeds[0]);
                for (i = 1; i < intf->op_state.n_speeds; i++) {
                    ds_put_format(ds, ", %d", intf->op_state.speeds[i]);
                }
            } else {
                ds_put_format(ds, "unset");
            }
            ds_put_format(ds, "\n");

            ds_put_format(ds, "    cfg_mtu            : %d\n",
                          intf->user_cfg.mtu);
            ds_put_format(ds, "    cfg_pause          : %d\n",
                          intf->user_cfg.pause);
            ds_put_format(ds, "    cfg_duplex         : %d\n",
                          intf->user_cfg.duplex);
            ds_put_format(ds, "    op_connector       : %s\n",
                          interface_pm_info_connector_strings[intf->pm_info.connector]);
            ds_put_format(ds, "    hw_interface_type  : %s\n",
                          intfd_get_intf_type_str(intf->pm_info.intf_type));
            ds_put_format(ds, "    lane_split         : %s\n",
                          intfd_get_lane_split_str(intf->user_cfg.lane_split));
            ds_put_format(ds, "    split_parent       : %s\n",
                          intf->split_parent ?
                          intf->split_parent->name : "none");
            if (!intf->split_children) {
                ds_put_format(ds, "    split_children     : none\n");
            } else {
                for (i = 0; i < intf->n_split_children; i++) {
                    ds_put_format(ds, "    split_children[%d]  : %s\n", i,
                                  intf->split_children[i] ?
                                  intf->split_children[i]->name : "not found");
                }
            }
        }
    }

} /* intfd_debug_dump */

static uint64_t
get_connector_flags(enum ovsrec_interface_pm_info_connector_e connector)
{
    switch (connector) {
    case INTERFACE_PM_INFO_CONNECTOR_SFP_RJ45:
    case INTERFACE_PM_INFO_CONNECTOR_SFP_SX:
        return PM_SFP_FLAGS;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_SFP_SR:
    case INTERFACE_PM_INFO_CONNECTOR_SFP_LR:
    case INTERFACE_PM_INFO_CONNECTOR_SFP_DAC:
        return PM_SFP_PLUS_FLAGS;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_SFP28_SR:
    case INTERFACE_PM_INFO_CONNECTOR_SFP28_LR:
    case INTERFACE_PM_INFO_CONNECTOR_SFP28_CR:
        return PM_SFP28_25G_FLAGS;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_QSFP_CR4:
    case INTERFACE_PM_INFO_CONNECTOR_QSFP_SR4:
    case INTERFACE_PM_INFO_CONNECTOR_QSFP_LR4:
        return PM_QSFP_PLUS_40G_FLAGS;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_QSFP28_SR4:
    case INTERFACE_PM_INFO_CONNECTOR_QSFP28_LR4:
    case INTERFACE_PM_INFO_CONNECTOR_QSFP28_CR4:
        return PM_QSFP28_100G_FLAGS;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_ABSENT:
    case INTERFACE_PM_INFO_CONNECTOR_UNKNOWN:
    case INTERFACE_PM_INFO_CONNECTOR_SFP_LX:
    case INTERFACE_PM_INFO_CONNECTOR_SFP_CX:
    case INTERFACE_PM_INFO_CONNECTOR_SFP_FC:
    case INTERFACE_PM_INFO_CONNECTOR_SFP_LRM:
    case INTERFACE_PM_INFO_CONNECTOR_QSFP28_PSM4:
    case INTERFACE_PM_INFO_CONNECTOR_QSFP28_CWDM4:
    case INTERFACE_PM_INFO_CONNECTOR_QSFP28_CLR4:
    default:
        return PM_UNSUPPORTED_FLAG;
        break;
    }
} /* get_connector_flags */

static enum ovsrec_interface_hw_intf_config_interface_type_e
get_connector_if_type(enum ovsrec_interface_pm_info_connector_e connector)
{
    switch (connector) {
    case INTERFACE_PM_INFO_CONNECTOR_SFP_SX:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_1GBASE_SX;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_SFP_RJ45:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_1GBASE_T;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_SFP_DAC:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_10GBASE_CR;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_SFP_SR:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_10GBASE_SR;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_SFP_LR:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_10GBASE_LR;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_QSFP_CR4:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_40GBASE_CR4;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_QSFP_SR4:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_40GBASE_SR4;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_QSFP_LR4:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_40GBASE_LR4;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_SFP28_CR:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_25GBASE_CR;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_SFP28_SR:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_25GBASE_SR;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_SFP28_LR:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_25GBASE_LR;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_QSFP28_CR4:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_100GBASE_CR4;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_QSFP28_SR4:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_100GBASE_SR4;
        break;
    case INTERFACE_PM_INFO_CONNECTOR_QSFP28_LR4:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_100GBASE_LR4;
        break;
    default:
        return INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_UNKNOWN;
        break;
    }
} /* get_connector_if_type */

/* Create a connection to the OVSDB at db_path and create a dB cache
 * for this daemon. */
void
intfd_ovsdb_init(const char *db_path)
{
    /* Initialize IDL through a new connection to the dB. */
    idl = ovsdb_idl_create(db_path, &ovsrec_idl_class, false, true);
    idl_seqno = ovsdb_idl_get_seqno(idl);
    ovsdb_idl_set_lock(idl, "ops_intfd");

    /* Reject writes to columns which are not marked write-only using
     * ovsdb_idl_omit_alert(). */
    ovsdb_idl_verify_write_only(idl);

    /* Choose some OVSDB tables and columns to cache. */
    ovsdb_idl_add_table(idl, &ovsrec_table_system);
    ovsdb_idl_add_table(idl, &ovsrec_table_subsystem);
    ovsdb_idl_add_table(idl, &ovsrec_table_interface);
    ovsdb_idl_add_table(idl, &ovsrec_table_port);

    /* Monitor the following columns, marking them read-only. */
    ovsdb_idl_add_column(idl, &ovsrec_system_col_cur_cfg);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_other_info);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_user_config);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_pm_info);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_split_parent);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_split_children);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_type);

    ovsdb_idl_add_column(idl, &ovsrec_interface_col_hw_intf_info);
    ovsdb_idl_omit_alert(idl, &ovsrec_interface_col_hw_intf_info);

    /* Mark the following columns write-only. */
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_error);
    ovsdb_idl_omit_alert(idl, &ovsrec_interface_col_error);

    ovsdb_idl_add_column(idl, &ovsrec_interface_col_hw_intf_config);
    ovsdb_idl_omit_alert(idl, &ovsrec_interface_col_hw_intf_config);

    ovsdb_idl_add_column(idl, &ovsrec_port_col_admin);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_interfaces);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_name);
} /* intfd_ovsdb_init */

void
intfd_ovsdb_exit(void)
{
    struct shash_node *sh_node = NULL, *sh_next = NULL;

    SHASH_FOR_EACH_SAFE(sh_node, sh_next, &all_interfaces) {
        del_old_interface(sh_node);
    }
    ovsdb_idl_destroy(idl);
} /* intfd_ovsdb_exit */

static bool
is_a_number(const char *nbr)
{
    while (*nbr) {
        if (!isdigit(*nbr++)) {
            return (false);
        }
    }

    return (true);

} /* is_a_number */

static int
parse_speeds(const char *speeds_str, uint32_t *speeds)
{
    char *ptr;
    char *sp;
    int i = 0;

    sp = xstrdup(speeds_str);  /* strtok doesn't work with const char * */

    ptr = strtok(sp, ",");
    while (ptr) {
        if (i >= INTFD_MAX_SPEEDS_ALLOWED || !is_a_number(
                                                (const char *) ptr)) {
            i = -1;
            break;
        }
        speeds[i] = atoi(ptr);
        i++;
        ptr = strtok(NULL, ",");
    }

    free(sp);
    return (i);

} /* parse_speeds */

/* Function : get_matching_port_row()
 * Desc     : search the ovsdb and get the matching
 *            port row based on the interface row name.
 * Param    : seach based on row name
 * Return   : returns the matching row or NULL incase
 *            no row is found.
 */
struct ovsrec_port *
get_matching_port_row(const char *name)
{
    const struct ovsrec_port *port_row = NULL;
    const struct ovsrec_interface *intf_row = NULL;
    int i;

    /* find out which port has this interface associated with it */
    OVSREC_PORT_FOR_EACH(port_row, idl) {
        if (port_row->n_interfaces) {
            for (i = 0; i < port_row->n_interfaces; i++) {
                intf_row = port_row->interfaces[i];
                if (!strcmp(intf_row->name, name)) {
                    return (struct ovsrec_port *)port_row;
                }
            }
        }
    }

    return NULL;
}

static int
port_parse_admin(enum ovsrec_port_config_admin_e  *port_admin,
                 const struct ovsrec_interface *ifrow)
{
    const struct ovsrec_port *port_row = NULL;
    *port_admin = PORT_ADMIN_CONFIG_DOWN;
    int rc = 0;

    port_row = get_matching_port_row(ifrow->name);
    if(port_row) {
        if ((port_row->admin == NULL) || (strcmp(port_row->admin, "up") == 0)) {
            *port_admin = PORT_ADMIN_CONFIG_UP;
        } else {
            *port_admin = PORT_ADMIN_CONFIG_DOWN;
        }
        rc++;
    }
    return rc;
}

static enum ovsrec_interface_user_config_admin_e
intf_parse_admin(const struct ovsrec_interface *intf_row) {
    enum ovsrec_interface_user_config_admin_e rc = INTERFACE_USER_CONFIG_ADMIN_DOWN;
    const char *data = NULL;

    data = smap_get((const struct smap *)&intf_row->user_config,
            INTERFACE_USER_CONFIG_MAP_ADMIN);
    if (data && (STR_EQ(data, OVSREC_INTERFACE_USER_CONFIG_ADMIN_UP))) {
        rc = INTERFACE_USER_CONFIG_ADMIN_UP;
    }
    return rc;
}

static void
intfd_parse_hw_info(struct intf_hw_info *hw_info,
                    const struct smap *ifrow_hw_info)
{
    const char *data = NULL;

    /* hw_info:pluggable */
    hw_info->is_pluggable = false;

    /* Check if the interface is pluggable. */
    data = smap_get(ifrow_hw_info, INTERFACE_HW_INTF_INFO_MAP_PLUGGABLE);
    if (data && (STR_EQ(data, INTERFACE_HW_INTF_INFO_MAP_PLUGGABLE_TRUE))) {
        hw_info->is_pluggable = true;
    }

    /* hw_info:connector */
    hw_info->connector = INTERFACE_HW_INTF_INFO_CONNECTOR_UNKNOWN;

    /* Check the connector type. */
    data = smap_get(ifrow_hw_info, INTERFACE_HW_INTF_INFO_MAP_CONNECTOR);
    if (data && (STR_EQ(data, INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_RJ45))) {
        hw_info->connector = INTERFACE_HW_INTF_INFO_CONNECTOR_RJ45;

    } else if (data && (STR_EQ(data, INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_SFP_PLUS))) {
        hw_info->connector = INTERFACE_HW_INTF_INFO_CONNECTOR_SFP_PLUS;

    } else if (data && (STR_EQ(data, INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_QSFP_PLUS))) {
        hw_info->connector = INTERFACE_HW_INTF_INFO_CONNECTOR_QSFP_PLUS;

    } else if (data && (STR_EQ(data, INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_QSFP28))) {
        hw_info->connector = INTERFACE_HW_INTF_INFO_CONNECTOR_QSFP28;

    }

    memset(hw_info->speeds, 0, sizeof(hw_info->speeds));
    hw_info->n_speeds = 0;
    data = smap_get(ifrow_hw_info, INTERFACE_HW_INTF_INFO_MAP_SPEEDS);
    if (data) {
        hw_info->n_speeds = parse_speeds(data, hw_info->speeds);
    }

    if (hw_info->n_speeds == 0) {
        VLOG_WARN("value for speeds not set in h/w description file");
    }

    hw_info->max_speed = 0;
    data = smap_get(ifrow_hw_info, INTERFACE_HW_INTF_INFO_MAP_MAX_SPEED);
    if (data) {
        hw_info->max_speed = atoi(data);
    }

    if (hw_info->max_speed == 0) {
        VLOG_WARN("value for max_speed not set in h/w description file");
    }

} /* intfd_parse_hw_info */

static void
intfd_parse_user_cfg(struct intf_user_cfg *user_config,
                     const struct smap *ifrow_config,
                     const struct smap *ifrow_hw_intf_info)
{
    const char *data = NULL;
    const char *hw_info_speeds = NULL;
    struct intf_hw_info hw_supported_speeds;

    VLOG_DBG("Updating user config\n");
    intfd_print_smap("interface_user_config", ifrow_config);

    /* FIXME: Add functions to validate the user_config data.
     * Without meta-schema we can't do such validation. */

    /* user_config:admin_state */
    user_config->admin_state = INTERFACE_USER_CONFIG_ADMIN_DOWN;

    data = smap_get(ifrow_config, INTERFACE_USER_CONFIG_MAP_ADMIN);
    if (data && (STR_EQ(data, OVSREC_INTERFACE_USER_CONFIG_ADMIN_UP))) {
        user_config->admin_state = INTERFACE_USER_CONFIG_ADMIN_UP;
    }

    /* user_config:autoneg */
    user_config->autoneg = INTERFACE_USER_CONFIG_AUTONEG_DEFAULT;

    data = smap_get(ifrow_config, INTERFACE_USER_CONFIG_MAP_AUTONEG);
    if (data && (STR_EQ(data, INTERFACE_USER_CONFIG_MAP_AUTONEG_OFF))) {
        user_config->autoneg = INTERFACE_USER_CONFIG_AUTONEG_OFF;

    } else if (data && (STR_EQ(data, INTERFACE_USER_CONFIG_MAP_AUTONEG_ON))) {
        user_config->autoneg = INTERFACE_USER_CONFIG_AUTONEG_ON;
    }

    /* user_config:pause */
    user_config->pause = INTERFACE_USER_CONFIG_PAUSE_NONE;

    data = smap_get(ifrow_config, INTERFACE_USER_CONFIG_MAP_PAUSE);
    if (data && (STR_EQ(data, INTERFACE_USER_CONFIG_MAP_PAUSE_RXTX))) {
        user_config->pause = INTERFACE_USER_CONFIG_PAUSE_RXTX;

    } else if (data && (STR_EQ(data, INTERFACE_USER_CONFIG_MAP_PAUSE_TX))) {
        user_config->pause = INTERFACE_USER_CONFIG_PAUSE_TX;

    } else if (data && (STR_EQ(data, INTERFACE_USER_CONFIG_MAP_PAUSE_RX))) {
        user_config->pause = INTERFACE_USER_CONFIG_PAUSE_RX;
    }

    /* user_config:duplex */
    user_config->duplex = INTERFACE_USER_CONFIG_DUPLEX_FULL;

    data = smap_get(ifrow_config, INTERFACE_USER_CONFIG_MAP_DUPLEX);
    if (data && (STR_EQ(data, INTERFACE_USER_CONFIG_MAP_DUPLEX_HALF))) {
        user_config->duplex = INTERFACE_USER_CONFIG_DUPLEX_HALF;
    }

    /* Get user supplied speeds which can be passed on to vswitchd.
     * data (user supplied speeds) is a comma separated list of numeric strings.
     * Need to verify user input against supported speeds list.
    */
    user_config->n_speeds = 0;
    data = smap_get(ifrow_config, INTERFACE_USER_CONFIG_MAP_SPEEDS);
    if (data) {
        int i, j, found;

        memset(user_config->speeds, 0, sizeof(user_config->speeds));
        /* parse_speeds() returns -1 if invalid user input */
        user_config->n_speeds =
                        parse_speeds(data, user_config->speeds);

        if (user_config->n_speeds > 0) {
            /* Get speeds from hw_info */
            hw_info_speeds = smap_get(ifrow_hw_intf_info,
                                      INTERFACE_HW_INTF_INFO_MAP_SPEEDS);
            hw_supported_speeds.n_speeds = parse_speeds(hw_info_speeds,
                                           hw_supported_speeds.speeds);
            if (user_config->n_speeds <= hw_supported_speeds.n_speeds) {
                for (i = 0; i < user_config->n_speeds; i++) {
                    found = false;
                    for (j = 0; j < hw_supported_speeds.n_speeds; j++) {
                        if (user_config->speeds[i] ==
                            hw_supported_speeds.speeds[j]) {

                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        user_config->n_speeds = -1;
                        break;
                    }
                }
            } else {
                user_config->n_speeds = -1;
            }
        }
    }

    user_config->mtu = 0;
    data = smap_get(ifrow_config, INTERFACE_USER_CONFIG_MAP_MTU);
    if (data) {
        user_config->mtu = -1;
        if (is_a_number(data)) {
            user_config->mtu = atoi(data);
            if ((user_config->mtu < INTFD_MIN_ALLOWED_USER_SPECIFIED_MTU) ||
                (user_config->mtu > base_subsys.mtu)) {

                user_config->mtu = -1;
            }
        }
    }

    /* user_config:lane_split */
    user_config->lane_split = INTERFACE_USER_CONFIG_LANE_SPLIT_NO_SPLIT;
    data = smap_get(ifrow_config, INTERFACE_USER_CONFIG_MAP_LANE_SPLIT);
    if (data && (STR_EQ(data, INTERFACE_USER_CONFIG_MAP_LANE_SPLIT_SPLIT))) {
        user_config->lane_split = INTERFACE_USER_CONFIG_LANE_SPLIT_SPLIT;
    }

} /* intfd_parse_user_cfg */

static void
intfd_parse_split_pm_info(struct intf_pm_info *pm_info, const struct smap *ifrow_pm_info)
{
    const char *data = NULL;

    /* pm_info:connector_status */
    pm_info->connector_status = INTERFACE_PM_INFO_CONNECTOR_STATUS_UNRECOGNIZED;

    data = smap_get(ifrow_pm_info, INTERFACE_PM_INFO_MAP_CONNECTOR_STATUS);
    if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_STATUS_UNSUPPORTED))) {
        pm_info->connector_status = INTERFACE_PM_INFO_CONNECTOR_STATUS_UNSUPPORTED;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_STATUS_SUPPORTED))) {
        pm_info->connector_status = INTERFACE_PM_INFO_CONNECTOR_STATUS_SUPPORTED;
    }

    data = smap_get(ifrow_pm_info, INTERFACE_PM_INFO_MAP_CONNECTOR);
    if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP28_CR4))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP28_CR;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP28_LR4))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP28_LR;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP28_SR4))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP28_SR;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP_CR4))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_DAC;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP_LR4))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_LR;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP_SR4))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_SR;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_ABSENT))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_ABSENT;

    } else {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_UNKNOWN;
        pm_info->connector_status = INTERFACE_PM_INFO_CONNECTOR_STATUS_UNSUPPORTED;
    }

    pm_info->op_connector_flags = get_connector_flags(pm_info->connector);
    pm_info->intf_type = get_connector_if_type(pm_info->connector);

} /* intfd_parse_split_pm_info */

static void
intfd_parse_pm_info(struct intf_hw_info *hw_info, struct intf_pm_info *pm_info,
                    const struct smap *ifrow_pm_info)
{
    const char *data = NULL;

    /* If the interface is a fixed port (non-pluggable). */
    if (hw_info->is_pluggable == false) {

        pm_info->connector_status = INTERFACE_PM_INFO_CONNECTOR_STATUS_SUPPORTED;

        /* Currently intfd only cares about non-pluggable fixed ports of type RJ45.
         * All other connector types are pluggable, and for them
         * pm_info will give the details about the pluggable module.
         */
        if (hw_info->connector == INTERFACE_HW_INTF_INFO_CONNECTOR_RJ45) {
            pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_RJ45;

        } else {
            pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_UNKNOWN;
            pm_info->connector_status = INTERFACE_PM_INFO_CONNECTOR_STATUS_UNRECOGNIZED;
        }

        pm_info->op_connector_flags = get_connector_flags(pm_info->connector);
        pm_info->intf_type = get_connector_if_type(pm_info->connector);

        return;
    }


    /* pm_info:connector_status */
    pm_info->connector_status = INTERFACE_PM_INFO_CONNECTOR_STATUS_UNRECOGNIZED;

    data = smap_get(ifrow_pm_info, INTERFACE_PM_INFO_MAP_CONNECTOR_STATUS);
    if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_STATUS_UNSUPPORTED))) {
        pm_info->connector_status = INTERFACE_PM_INFO_CONNECTOR_STATUS_UNSUPPORTED;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_STATUS_SUPPORTED))) {
        pm_info->connector_status = INTERFACE_PM_INFO_CONNECTOR_STATUS_SUPPORTED;
    }

    /* pm_info:connector */
    pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_UNKNOWN;

    data = smap_get(ifrow_pm_info, INTERFACE_PM_INFO_MAP_CONNECTOR);
    if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP28_CR4))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_QSFP28_CR4;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP28_LR4))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_QSFP28_LR4;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP28_SR4))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_QSFP28_SR4;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP28_CR))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP28_CR;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP28_LR))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP28_LR;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP28_SR))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP28_SR;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP_CR4))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_QSFP_CR4;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP_LR4))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_QSFP_LR4;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_QSFP_SR4))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_QSFP_SR4;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_CX))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_CX;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_DAC))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_DAC;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_FC))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_FC;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_LR))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_LR;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_LRM))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_LRM;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_LX))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_LX;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_RJ45))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_RJ45;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_SR))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_SR;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_SFP_SX))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_SFP_SX;

    } else if (data && (STR_EQ(data, OVSREC_INTERFACE_PM_INFO_CONNECTOR_ABSENT))) {
        pm_info->connector = INTERFACE_PM_INFO_CONNECTOR_ABSENT;
    }

    pm_info->op_connector_flags = get_connector_flags(pm_info->connector);
    pm_info->intf_type = get_connector_if_type(pm_info->connector);

} /* intfd_parse_pm_info */

static void
intfd_process_parent_child(struct iface *intf,
                           const struct ovsrec_interface *ifrow)
{
    int i;

    /* For an interface that have a parent or children, update
     * the interface data with a pointer to the parent or children.
     */
    if (!ifrow->split_parent && !ifrow->split_children) {
        /* Not a splittable port.  Nothing to do. */
        return;
    }

    /* Handle parent pointer */
    if (ifrow->split_parent) {
        intf->split_parent = shash_find_data(&all_interfaces,
                                             ifrow->split_parent->name);
        if (!intf->split_parent) {
            VLOG_WARN("Could not find parent ifrow->name %s in "
                      "all_interfaces!", ifrow->split_parent->name);
            return;
        }

        /* Children ports take PM info from their parent. */
        intfd_parse_split_pm_info(&(intf->pm_info), &(ifrow->split_parent->pm_info));

    /* Handle children pointers */
    } else if (ifrow->split_children) {
        struct iface *if_child_p;

        intf->split_children = xcalloc(ifrow->n_split_children,
                                       sizeof(struct iface *));
        for (i = 0; i < ifrow->n_split_children; i++) {
            if_child_p = shash_find_data(&all_interfaces,
                                         ifrow->split_children[i]->name);
            if (!if_child_p) {
                VLOG_WARN("Could not find child ifrow->name %s in "
                          "all_interfaces!", ifrow->split_children[i]->name);
                intf->split_children[i] = NULL;
                continue;
            }
            intf->split_children[i] = if_child_p;
        }
        intf->n_split_children = ifrow->n_split_children;
    }

} /* intfd_process_parent_child */

static void
set_op_state_pause(struct iface *intf)
{
    enum ovsrec_interface_hw_intf_config_pause_e hw_pause;

    switch(intf->user_cfg.pause) {

    case INTERFACE_USER_CONFIG_PAUSE_RXTX:
        hw_pause = INTERFACE_HW_INTF_CONFIG_PAUSE_RXTX;
        break;

    case INTERFACE_USER_CONFIG_PAUSE_TX:
        hw_pause = INTERFACE_HW_INTF_CONFIG_PAUSE_TX;
        break;

    case INTERFACE_USER_CONFIG_PAUSE_RX:
        hw_pause = INTERFACE_HW_INTF_CONFIG_PAUSE_RX;
        break;

    case INTERFACE_USER_CONFIG_PAUSE_NONE:
    default:
        hw_pause = INTERFACE_HW_INTF_CONFIG_PAUSE_NONE;
        break;
    }
    intf->op_state.pause = hw_pause;

} /* set_op_state_pause */

static uint32_t
intfd_highest_speed(uint32_t *speeds, int num_speeds)
{
    uint32_t speed;
    int i;

    if (num_speeds < 1) {
        return 0;
    }

    speed = speeds[0];
    for (i = 1; i < num_speeds; i++) {
        if (speeds[i] > speed) {
            speed = speeds[i];
        }
    }

    return (speed);

} /* intfd_highest_speed */

static void
set_op_state_duplex(struct iface *intf)
{
    enum ovsrec_interface_hw_intf_config_duplex_e hw_duplex;

    switch(intf->user_cfg.duplex) {

    case INTERFACE_USER_CONFIG_DUPLEX_HALF:
        hw_duplex = INTERFACE_HW_INTF_CONFIG_DUPLEX_HALF;
        break;

    case INTERFACE_USER_CONFIG_DUPLEX_FULL:
    default:
       hw_duplex = INTERFACE_HW_INTF_CONFIG_DUPLEX_FULL;
        break;
    }
    intf->op_state.duplex = hw_duplex;

} /* set_op_state_duplex */

static void
add_new_port(const struct ovsrec_port *port_row)
{
    struct port_info *new_port = NULL;
    int i;

    VLOG_DBG("Port %s being added!\n", port_row->name);

    /* If the port already exists, return. */
    if (NULL != shash_find(&all_ports, port_row->name)) {
        VLOG_WARN("Interface %s specified twice", port_row->name);
        return;
    }

    /* Allocate structure to save state information for this port. */
    new_port = xzalloc(sizeof(struct port_info));

    shash_add(&all_ports, port_row->name, new_port);

    new_port->name = xstrdup(port_row->name);
    new_port->interface = xmalloc(port_row->n_interfaces * sizeof(struct ovsrec_interface *));
    new_port->n_interfaces = port_row->n_interfaces;
    for (i = 0; i < port_row->n_interfaces; i++) {
        new_port->interface[i] = port_row->interfaces[i];
    }

    VLOG_DBG("Created local data structure for port %s", port_row->name);

} /* add_new_port */

static void
add_new_interface(const struct ovsrec_interface *ifrow)
{
    struct iface *new_intf = NULL;

    VLOG_DBG("Interface %s being added!\n", ifrow->name);

    /* If the interface already exists, return. */
    if (NULL != shash_find(&all_interfaces, ifrow->name)) {
        VLOG_WARN("Interface %s specified twice", ifrow->name);
        return;
    }

    /* Allocate structure to save state information for this interface. */
    new_intf = xzalloc(sizeof *new_intf);

    shash_add(&all_interfaces, ifrow->name, new_intf);

    new_intf->name = xstrdup(ifrow->name);

    intfd_parse_hw_info(&(new_intf->hw_info), &(ifrow->hw_intf_info));
    intfd_parse_user_cfg(&(new_intf->user_cfg), &(ifrow->user_config),
                         &(ifrow->hw_intf_info));

    new_intf->type = xstrdup(ifrow->type);

    /* Check for pm_info only if the interface is not internal */
    intfd_parse_pm_info(&(new_intf->hw_info), &(new_intf->pm_info), &(ifrow->pm_info));
    port_parse_admin(&(new_intf->port_admin), ifrow);

    /* Note: splittable port processing occurs later once
     *       all interfaces have been added. */

    VLOG_DBG("Created local data structure for interface %s", ifrow->name);

} /* add_new_interface */

static void
del_old_interface(struct shash_node *sh_node)
{
    if (sh_node) {
        struct iface *intf = sh_node->data;
        free(intf->name);
        free(intf->type);
        if (intf->split_children) {
            free(intf->split_children);
        }
        free(intf);
        shash_delete(&all_interfaces, sh_node);
    }
} /* del_old_interface */

static void
del_old_port(struct shash_node *sh_node)
{
    int j;
    const struct ovsrec_interface *intf_row = NULL;
    struct smap hw_cfg_smap;

    if (sh_node) {
        struct port_info *port_data = sh_node->data;
        for(j = 0; j < port_data->n_interfaces; j++) {
            smap_init(&hw_cfg_smap);
            intf_row = port_data->interface[j];
            /* Making sure not to reset an interface associated with another port */
            if (!get_matching_port_row(intf_row->name))
            {
                VLOG_DBG("Port delete : reset interface %s\n", intf_row->name);
                smap_add(&hw_cfg_smap, INTERFACE_HW_INTF_CONFIG_MAP_ENABLE, INTERFACE_HW_INTF_CONFIG_MAP_ENABLE_FALSE);
                ovsrec_interface_set_hw_intf_config(intf_row, &hw_cfg_smap);
            }
            smap_destroy(&hw_cfg_smap);
        }
        free(port_data->name);
        free(port_data->interface);
        free(port_data);
        shash_delete(&all_ports, sh_node);
    }
} /* del_old_port */

/* FUNCTION: calc_intf_op_state_n_reason()
 *
 * This function determines an interface's operational state & reasons:
 *
 *    - op state:        Interface "hw_intf_config:enable"
 *    - op state reason: Interface "error"
 *
 * Following is a complete summary of the different operational states
 * and the associated reasons for an interface, listed in order of
 * priority, with the highest priority values listed first.
 *
 * Note that if multiple reasons apply to an interface, only the
 * highest priority reason is displayed.  E.g., if a pluggable
 * interface does not have any pluggable module installed, and its
 * interface admin state is set to disabled by an administrator, then
 * "error" will only show "admin_down".  It becomes "unpopulated"
 * after its admin interface state is set to enabled.
 *
 * OP STATE  OP STATE REASON        NOTES
 * --------  ---------------        -----
 *
 * disabled  lanes_split
 *                  Interface is not usable due to the interface being
 *                  configured in split mode.  Applicable to
 *                  splittable primary interface only (e.g. QSFP+, QSFP28).
 *
 * disabled  lanes_not_split
 *                  Interface is not usable due to the interface being
 *                  configured in non-split mode. Applicable to
 *                  splittable subinterfaces only (e.g. QSFP+, QSFP28).
 *
 * disabled  admin_down
 *                  Interface "user_config:admin" property is set to
 *                  "down" by an administrator.
 *
 * disabled  module_missing
 *                  Interface is pluggable, but no pluggable module is
 *                  present.
 *
 * disabled  module_unrecognized
 *                  Interface is pluggable, but the pluggable module
 *                  identification information cannot be read.
 *
 * disabled  module_unsupported
 *                  Interface is pluggable, but the pluggable module
 *                  type is not supported.  This may be one of the
 *                  following reasons:
 *                   1. Unsupported module.
 *                   2. Enet 1/10/40G modules in an interface that
 *                      does not support that particular Ethernet
 *                      speed.
 *
 * disabled  invalid_mtu
 *                  User specified MTU is invalid.
 *
 * disabled  invalid_speeds
 *                  User specified speeds is invalid.
 *
 * disabled  autoneg_required
 *                  User specified autoneg=off when it is required
 *
 * disabled  autoneg_not_supported
 *                  User specified autoneg=on when it is not supported
 *
 * NOTE: All new checks should be added above the following
 *
 * enabled   ok
 *                  A supported module is present and everything is
 *                  fine.  Interface is enabled.  This does not imply
 *                  link is necessarily up; it simply means a
 *                  interface is enabled at the h/w level.
 */
static void
calc_intf_op_state_n_reason(struct iface *intf)
{
    VLOG_DBG("Checking interface %s in hardware.\n", intf->name);

    /* Default the interface to disabled. */
    intf->op_state.enabled = false;
    intf->op_state.reason = INTERFACE_ERROR_UNINITIALIZED;

    if ((STR_EQ(intf->type, OVSREC_INTERFACE_TYPE_INTERNAL))
       || (STR_EQ(intf->type, OVSREC_INTERFACE_TYPE_VLANSUBINT))
       || (STR_EQ(intf->type, OVSREC_INTERFACE_TYPE_LOOPBACK))) {
        if (intf->user_cfg.admin_state == INTERFACE_USER_CONFIG_ADMIN_DOWN) {
            intf->op_state.reason = INTERFACE_ERROR_ADMIN_DOWN;

        /* Checking for port admin as down */
        } else if (intf->port_admin == PORT_ADMIN_CONFIG_DOWN) {
            intf->op_state.reason = PORT_ERROR_ADMIN_DOWN;
        } else {
            intf->op_state.enabled = true;
            intf->op_state.reason = INTERFACE_ERROR_OK;
        }
        return;
    }

    /* Checking for splittable primary interface & lanes_split condition. */
    if (intf->split_children != NULL &&
        intf->user_cfg.lane_split == INTERFACE_USER_CONFIG_LANE_SPLIT_SPLIT) {
        intf->op_state.reason = INTERFACE_ERROR_LANES_SPLIT;

    /* Checking for splittable subinterface & lanes_not_split condition. */
    } else if (intf->split_parent != NULL &&
               intf->split_parent->user_cfg.lane_split == INTERFACE_USER_CONFIG_LANE_SPLIT_NO_SPLIT) {
        intf->op_state.reason = INTERFACE_ERROR_LANES_NOT_SPLIT;

    /* Checking admin state. */
    } else if (intf->user_cfg.admin_state == INTERFACE_USER_CONFIG_ADMIN_DOWN) {
        intf->op_state.reason = INTERFACE_ERROR_ADMIN_DOWN;
        VLOG_DBG("calc_state: Set admin state to DOWN\n");

    /* Checking for missing pluggable module. */
    } else if (intf->pm_info.connector == INTERFACE_PM_INFO_CONNECTOR_ABSENT) {
        intf->op_state.reason = INTERFACE_ERROR_MODULE_MISSING;

    /* Checking for unrecognized pluggable module. */
    } else if (intf->pm_info.connector_status == INTERFACE_PM_INFO_CONNECTOR_STATUS_UNRECOGNIZED) {
        intf->op_state.reason = INTERFACE_ERROR_MODULE_UNRECOGNIZED;

    /* Checking for unsupported pluggable module. */
    } else if ((intf->pm_info.connector_status ==
                    INTERFACE_PM_INFO_CONNECTOR_STATUS_UNSUPPORTED) ||
               (intf->pm_info.op_connector_flags == PM_UNSUPPORTED_FLAG)) {
        intf->op_state.reason = INTERFACE_ERROR_MODULE_UNSUPPORTED;

    /* Checking for invalid mtu. */
    } else if (intf->op_state.mtu == -1) {
        intf->op_state.reason = INTERFACE_ERROR_INVALID_MTU;

    /* Checking for invalid speeds. */
    } else if (intf->op_state.n_speeds == -1) {
        intf->op_state.reason = INTERFACE_ERROR_INVALID_SPEEDS;

    /* Checking for invalid autoneg. */
    } else if (intf->op_state.autoneg_state == INTFD_AUTONEG_STATE_INVALID) {
        intf->op_state.reason = intf->op_state.autoneg_reason;

    /* Checking for port admin as down */
    } else if (intf->port_admin == PORT_ADMIN_CONFIG_DOWN) {
        intf->op_state.reason = PORT_ERROR_ADMIN_DOWN;

    } else {
        /* FIXME: Lots of other business logic needs to be added here. */
        /* If we get here, everything's fine. */

        intf->op_state.enabled = true;
        intf->op_state.reason = INTERFACE_ERROR_OK;
        VLOG_DBG("Need to enable interface %s in hardware.\n", intf->name);

    }

} /* calc_intf_op_state_n_reason */

static void
validate_n_set_interface_capability(struct iface *intf)
{
    /*
     * This function determines if an interface supports, requires, or does not
     * support auto-negotiation along with the supported speeds.
     * It then looks at user input for AN and speeds and either reports an
     * error if invalid user input or sets the appropriate value(s) for
     * AN and speeds.
    */

    /* FIXME: Add support for multi-speed capable transceivers (or
       fixed ports). Hard coded for now to single speed.
    */

    /* If the user input an invalid "speeds", return */
    if (intf->user_cfg.n_speeds == -1) {
        intf->op_state.n_speeds = -1;
        return;
    }

    /* If the user input an invalid mtu, return */
    if (intf->user_cfg.mtu == -1) {
        return;
    }

    if (CONNECTOR_IS_SFP_PLUS_10G(intf)) {
        intf->op_state.autoneg_capability = INTFD_AUTONEG_CAPABILITY_UNSUPPORTED;
        intf->op_state.speeds[0] = SPEED_10G;
        intf->op_state.n_speeds = 1;

    } else if (CONNECTOR_IS_SFP28_25G(intf)) {
        /* SFP28 CR requires AN, SR/LR do not. */
        if (INTERFACE_PM_INFO_CONNECTOR_SFP28_CR == intf->pm_info.connector) {
            intf->op_state.autoneg_capability = INTFD_AUTONEG_CAPABILITY_REQUIRED;
        } else {
            intf->op_state.autoneg_capability = INTFD_AUTONEG_CAPABILITY_UNSUPPORTED;
        }

        intf->op_state.speeds[0] = SPEED_25G;
        intf->op_state.n_speeds = 1;

    } else if (CONNECTOR_IS_SFP(intf)) {
        intf->op_state.autoneg_capability = INTFD_AUTONEG_CAPABILITY_REQUIRED;
        /* FIXME: Currently not supporting tri-speed devices */
        intf->op_state.speeds[0] = SPEED_1G;
        intf->op_state.n_speeds = 1;

    } else if (CONNECTOR_IS_QSFP_PLUS_40G(intf)) {
        /* QSFP+ CR4 requires AN, SR4/LR4 do not. */
        if (INTERFACE_PM_INFO_CONNECTOR_QSFP_CR4 == intf->pm_info.connector) {
            intf->op_state.autoneg_capability = INTFD_AUTONEG_CAPABILITY_REQUIRED;
        } else {
            intf->op_state.autoneg_capability = INTFD_AUTONEG_CAPABILITY_UNSUPPORTED;
        }

        intf->op_state.speeds[0] = SPEED_40G;
        intf->op_state.n_speeds = 1;

    } else if (CONNECTOR_IS_QSFP28_100G(intf)) {
        /* QSFP28 CR4 requires AN, SR4/LR4 do not. */
        if (INTERFACE_PM_INFO_CONNECTOR_QSFP28_CR4 == intf->pm_info.connector) {
            intf->op_state.autoneg_capability = INTFD_AUTONEG_CAPABILITY_REQUIRED;
        } else {
            intf->op_state.autoneg_capability = INTFD_AUTONEG_CAPABILITY_UNSUPPORTED;
        }

        intf->op_state.speeds[0] = SPEED_100G;
        intf->op_state.n_speeds = 1;

    } else {
        /* This should be midplane connectors.  As of now,
         * all midplane connections are of KR/KR2 variety,
         * which requires auto-negotiation, even if a
         * specific speed is specified later. */
        intf->op_state.autoneg_capability = INTFD_AUTONEG_CAPABILITY_REQUIRED;
        intf->op_state.speeds[0] = 0;
        intf->op_state.n_speeds = 0;
    }

    /* Override autoneg and speeds based on user input */
    intf->op_state.autoneg_reason = INTERFACE_ERROR_UNINITIALIZED;

    /* If autoneg=true and didn't set speeds */
    if ((intf->user_cfg.autoneg == INTERFACE_USER_CONFIG_AUTONEG_ON) &&
        (intf->user_cfg.n_speeds == 0)) {

        intf->op_state.autoneg_state = INTFD_AUTONEG_STATE_ENABLED;

        if (intf->op_state.autoneg_capability == INTFD_AUTONEG_CAPABILITY_UNSUPPORTED) {
            /* report error */
            intf->op_state.autoneg_state = INTFD_AUTONEG_STATE_INVALID;
            intf->op_state.autoneg_reason = INTERFACE_ERROR_AUTONEG_NOT_SUPPORTED;
        }

    /* If autoneg=false and didn't set speeds */
    } else if ((intf->user_cfg.autoneg == INTERFACE_USER_CONFIG_AUTONEG_OFF) &&
               (intf->user_cfg.n_speeds == 0)) {

        intf->op_state.autoneg_state = INTFD_AUTONEG_STATE_DISABLED;

        if (intf->op_state.autoneg_capability ==
                                        INTFD_AUTONEG_CAPABILITY_REQUIRED) {
            /* report error */
            intf->op_state.autoneg_state = INTFD_AUTONEG_STATE_INVALID;
            intf->op_state.autoneg_reason = INTERFACE_ERROR_AUTONEG_REQUIRED;

        } else if (intf->op_state.autoneg_capability ==
                                        INTFD_AUTONEG_CAPABILITY_OPTIONAL) {

            /* use highest supported speed */
            intf->op_state.speeds[0] = intfd_highest_speed(intf->hw_info.speeds,
                                                           intf->hw_info.n_speeds);
            intf->op_state.n_speeds = 1;
        }

    /* If not set autoneg and set speeds */
    } else if ((intf->user_cfg.autoneg == INTERFACE_USER_CONFIG_AUTONEG_DEFAULT) &&
               (intf->user_cfg.n_speeds > 0)) {

        if (intf->op_state.autoneg_capability != INTFD_AUTONEG_CAPABILITY_UNSUPPORTED) {

            intf->op_state.autoneg_state = INTFD_AUTONEG_STATE_ENABLED;

            /* Use user speeds */
            memcpy(intf->op_state.speeds, intf->user_cfg.speeds,
                   sizeof(intf->user_cfg.speeds));
            intf->op_state.n_speeds = intf->user_cfg.n_speeds;

        } else {

            intf->op_state.autoneg_state = INTFD_AUTONEG_STATE_DISABLED;

            /* get first speed supplied by user */
            intf->op_state.speeds[0] = intf->user_cfg.speeds[0];
            intf->op_state.n_speeds = 1;
        }

    /* If autoneg=true and set speeds */
    } else if ((intf->user_cfg.autoneg == INTERFACE_USER_CONFIG_AUTONEG_ON) &&
               (intf->user_cfg.n_speeds > 0)) {

        if ((intf->op_state.autoneg_capability == INTFD_AUTONEG_CAPABILITY_REQUIRED) ||
            (intf->op_state.autoneg_capability == INTFD_AUTONEG_CAPABILITY_OPTIONAL)) {

            intf->op_state.autoneg_state = INTFD_AUTONEG_STATE_ENABLED;

            /* Use user speeds */
            memcpy(intf->op_state.speeds, intf->user_cfg.speeds,
                   sizeof(intf->user_cfg.speeds));
            intf->op_state.n_speeds = intf->user_cfg.n_speeds;

        } else {
            /* report error */
            intf->op_state.autoneg_state = INTFD_AUTONEG_STATE_INVALID;
            intf->op_state.autoneg_reason = INTERFACE_ERROR_AUTONEG_NOT_SUPPORTED;
        }

    /* If autoneg=false and set speeds */
    } else if ((intf->user_cfg.autoneg == INTERFACE_USER_CONFIG_AUTONEG_OFF) &&
               (intf->user_cfg.n_speeds > 0)) {

        intf->op_state.autoneg_state = INTFD_AUTONEG_STATE_DISABLED;

        if (intf->op_state.autoneg_capability == INTFD_AUTONEG_CAPABILITY_REQUIRED) {

            /* report error */
            intf->op_state.autoneg_state = INTFD_AUTONEG_STATE_INVALID;
            intf->op_state.autoneg_reason = INTERFACE_ERROR_AUTONEG_REQUIRED;
        } else {
            /* Use first entry in user speeds */
            intf->op_state.speeds[0] = intf->user_cfg.speeds[0];
            intf->op_state.n_speeds = 1;
        }

    /* If not set autoneg and not set speeds */
    } else {
        if (intf->op_state.autoneg_capability == INTFD_AUTONEG_CAPABILITY_UNSUPPORTED) {
            intf->op_state.autoneg_state = INTFD_AUTONEG_STATE_DISABLED;
        } else {
            intf->op_state.autoneg_state = INTFD_AUTONEG_STATE_ENABLED;
        }
    }

} /* validate_n_set_interface_capability */

void
set_intf_hw_config_in_db(const struct ovsrec_interface *ifrow, struct iface *intf)
{
    const char *tmp_str = NULL;

    struct smap smap = SMAP_INITIALIZER(&smap);

    /* Write H/W config changes to the interface row in OVSDB. */
    tmp_str = NULL;
    if (intf->op_state.enabled != true) {
        tmp_str = intfd_get_error_str(intf->op_state.reason);
    }
    ovsrec_interface_set_error(ifrow, tmp_str);

    /* We want to build up a new hw_intf_config map. */

    /* hw_intf_config:enabled */
    tmp_str = INTERFACE_HW_INTF_CONFIG_MAP_ENABLE_FALSE;
    if (intf->op_state.enabled == true) {
        tmp_str = INTERFACE_HW_INTF_CONFIG_MAP_ENABLE_TRUE;
    }

    smap_add(&smap, INTERFACE_HW_INTF_CONFIG_MAP_ENABLE, tmp_str);

    if ((intf->op_state.enabled == true) &&
        ((!STR_EQ(intf->type, OVSREC_INTERFACE_TYPE_INTERNAL)) ||
        (!STR_EQ(intf->type, OVSREC_INTERFACE_TYPE_VLANSUBINT))  ||
        (!STR_EQ(intf->type, OVSREC_INTERFACE_TYPE_LOOPBACK))))  {

        /* hw_intf_config:autoneg */
        tmp_str = INTERFACE_HW_INTF_CONFIG_MAP_AUTONEG_OFF;
        if (intf->op_state.autoneg_state == INTFD_AUTONEG_STATE_ENABLED) {
            tmp_str = INTERFACE_HW_INTF_CONFIG_MAP_AUTONEG_ON;
        }

        smap_add(&smap, INTERFACE_HW_INTF_CONFIG_MAP_AUTONEG, tmp_str);

        /* hw_intf_config:duplex */
        tmp_str = INTERFACE_HW_INTF_CONFIG_MAP_DUPLEX_FULL;
        if (intf->op_state.duplex == INTERFACE_HW_INTF_CONFIG_DUPLEX_HALF) {
            tmp_str = INTERFACE_HW_INTF_CONFIG_MAP_DUPLEX_HALF;
        }

        smap_add(&smap, INTERFACE_HW_INTF_CONFIG_MAP_DUPLEX, tmp_str);

        /* hw_intf_config:pause */
        tmp_str = INTERFACE_HW_INTF_CONFIG_MAP_PAUSE_NONE;
        if (intf->op_state.pause == INTERFACE_HW_INTF_CONFIG_PAUSE_RXTX) {
            tmp_str = INTERFACE_HW_INTF_CONFIG_MAP_PAUSE_RXTX;

        } else if (intf->op_state.pause == INTERFACE_HW_INTF_CONFIG_PAUSE_TX) {
            tmp_str = INTERFACE_HW_INTF_CONFIG_MAP_PAUSE_TX;

        } else if (intf->op_state.pause == INTERFACE_HW_INTF_CONFIG_PAUSE_RX) {
            tmp_str = INTERFACE_HW_INTF_CONFIG_MAP_PAUSE_RX;
        }

        smap_add(&smap, INTERFACE_HW_INTF_CONFIG_MAP_PAUSE, tmp_str);

        /* hw_intf_config:mtu */
        if (intf->op_state.mtu >= INTFD_MIN_ALLOWED_USER_SPECIFIED_MTU) {
            smap_add_format(&smap, INTERFACE_HW_INTF_CONFIG_MAP_MTU, "%d",
                            intf->op_state.mtu);
        }

        /* Set speeds */
        if (intf->op_state.n_speeds > 0) {
            /* Use user-configured speeds. */
            char speed_string[INTFD_MAX_SPEEDS_ALLOWED*20];
            int i = 0;

            sprintf(speed_string, "%d", intf->op_state.speeds[0]);
            for (i = 1; i < intf->op_state.n_speeds; i++) {
                sprintf(speed_string+strlen(speed_string), ",%d",
                        intf->op_state.speeds[i]);
            }

            smap_add(&smap, INTERFACE_HW_INTF_CONFIG_MAP_SPEEDS, speed_string);
        }
        smap_add(&smap, INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE,
                  intfd_get_intf_type_str(intf->pm_info.intf_type));
    }

    ovsrec_interface_set_hw_intf_config(ifrow, &smap);

} /* set_intf_hw_config_in_db */

static void
set_op_state_mtu(struct iface *intf)
{
    /* Use the user MTU if specified and valid, else use default */
    switch (intf->user_cfg.mtu) {
        case -1:
            intf->op_state.mtu = -1;
            break;
        case 0:
            intf->op_state.mtu = INTFD_DEFAULT_MTU;
            break;
        default:
            intf->op_state.mtu = intf->user_cfg.mtu;
            break;
    }
} /* set_op_state_mtu */

void
set_interface_config(const struct ovsrec_interface *ifrow, struct iface *intf)
{
    VLOG_DBG("Received new config for interface %s", ifrow->name);

    /* Set mtu. */
    set_op_state_mtu(intf);

    /* Update autoneg capabilities of the interface. */
    validate_n_set_interface_capability(intf);

    /* Figure out if interface can be enabled. */
    calc_intf_op_state_n_reason(intf);

    if (intf->op_state.enabled == true) {

        set_op_state_pause(intf);

        set_op_state_duplex(intf);
    }

    /* One interface needs to be reconfigured in h/w. */
    set_intf_hw_config_in_db(ifrow, intf);

} /* set_interface_config */

static int
handle_interfaces_config_mods(struct shash *sh_idl_interfaces)
{
    int rc = 0;
    int i;
    bool cfg_changed = false;
    bool split_changed = false;
    bool pm_info_changed = false;
    struct intf_user_cfg new_user_cfg;
    struct intf_pm_info new_pm_info;
    struct shash_node *sh_node;
    struct iface *intf = NULL;
    const struct ovsrec_interface *ifrow = NULL;

    VLOG_DBG("handle_interfaces_config_mods\n");
    /* Loop through all the current interfaces and handle config changes. */
    SHASH_FOR_EACH(sh_node, &all_interfaces) {
        cfg_changed = false;
        intf = sh_node->data;
        ifrow = shash_find_data(sh_idl_interfaces, sh_node->name);

        if (OVSREC_IDL_IS_ROW_INSERTED(ifrow, idl_seqno)) {

            /* Update parent/child relationship if needed. */
            intfd_process_parent_child(intf, ifrow);

            set_interface_config(ifrow, intf);
            rc++;

        } else if (OVSREC_IDL_IS_ROW_MODIFIED(ifrow, idl_seqno)) {

            VLOG_DBG("Something got modified\n");
            intfd_parse_user_cfg(&new_user_cfg, &ifrow->user_config,
                                 &ifrow->hw_intf_info);

            port_parse_admin(&(intf->port_admin), ifrow);

            if (!ifrow->split_parent) {
                /* Parse this row's pm_info. */
                intfd_parse_pm_info(&(intf->hw_info), &new_pm_info, &(ifrow->pm_info));
            } else {
                /* Parse the parent's row's pm_info. */
                intfd_parse_split_pm_info(&new_pm_info, &(ifrow->split_parent->pm_info));
            }

            if (intf->user_cfg.admin_state != new_user_cfg.admin_state) {
                cfg_changed = true;
                intf->user_cfg.admin_state = new_user_cfg.admin_state;
            }

            if (intf->user_cfg.autoneg != new_user_cfg.autoneg) {
                cfg_changed = true;
                intf->user_cfg.autoneg = new_user_cfg.autoneg;
            }

            if (intf->user_cfg.pause != new_user_cfg.pause) {
                cfg_changed = true;
                intf->user_cfg.pause = new_user_cfg.pause;
            }

            if (intf->user_cfg.duplex != new_user_cfg.duplex) {
                cfg_changed = true;
                intf->user_cfg.duplex = new_user_cfg.duplex;
            }

            if (intf->user_cfg.mtu != new_user_cfg.mtu) {
                cfg_changed = true;
                intf->user_cfg.mtu = new_user_cfg.mtu;
            }

            for (i = 0; i < INTFD_MAX_SPEEDS_ALLOWED; i++) {
                if (intf->user_cfg.speeds[i] != new_user_cfg.speeds[i]) {
                    cfg_changed = true;
                    intf->user_cfg.speeds[i] = new_user_cfg.speeds[i];
                }
            }

            if (intf->user_cfg.n_speeds != new_user_cfg.n_speeds) {
                cfg_changed = true;
                intf->user_cfg.n_speeds = new_user_cfg.n_speeds;
            }

            if (intf->user_cfg.lane_split != new_user_cfg.lane_split) {
                cfg_changed = true;
                split_changed = true;
                intf->user_cfg.lane_split = new_user_cfg.lane_split;
            }

            if (intf->pm_info.connector != new_pm_info.connector) {
                cfg_changed = true;
                pm_info_changed = true;
                intf->pm_info.connector = new_pm_info.connector;
            }

            if (intf->pm_info.connector_status != new_pm_info.connector_status) {
                cfg_changed = true;
                intf->pm_info.connector_status = new_pm_info.connector_status;
            }

            if (intf->pm_info.intf_type != new_pm_info.intf_type) {
                cfg_changed = true;
                intf->pm_info.intf_type = new_pm_info.intf_type;
            }

            if (intf->pm_info.op_connector_flags != new_pm_info.op_connector_flags) {
                cfg_changed = true;
                intf->pm_info.op_connector_flags = new_pm_info.op_connector_flags;
            }

            VLOG_DBG("cfg_changed = %d\n", cfg_changed);
            if (cfg_changed) {
                /* Update interface configuration. */
                set_interface_config(ifrow, intf);
                rc++;
            }

            /* If parent port's connector is changed, pass on
             * the change to split children. */
            if (pm_info_changed && ifrow->split_children) {
                int i;
                for (i = 0; i < intf->n_split_children; i++) {
                    intfd_parse_split_pm_info(&(intf->split_children[i]->pm_info),
                                              &(ifrow->pm_info));
                }
                split_changed = true;
            }

            if (split_changed) {
                int i;
                /* Lane split status changed.  Need to
                 * reconfigure all split children as well. */
                for (i = 0; i < intf->n_split_children; i++) {
                    set_interface_config(ifrow->split_children[i],
                                         intf->split_children[i]);
                }
            }
        }
    }

    return rc;

} /* handle_interfaces_config_mods */

/* Function : add_del_interface_handle_port_config_mods()
 * Desc     : Updates the hw_config key "enable" based on the user
 *            configuration to set the port admin state to up or down.
 * Param    : None
 * Return   : None
 */
static int
add_del_interface_handle_port_config_mods(void)
{
    const struct ovsrec_port *port_row = NULL;
    const struct ovsrec_interface *intf_row = NULL;
    int i;
    struct iface *intf;
    int rc = 0;
    struct port_info *port_data;
    const char *data = NULL;

    VLOG_DBG("add_del_interface_handle_port_config_mods\n");

    if ((OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_port_col_admin,
                    idl_seqno)) ||
            (OVSREC_IDL_IS_COLUMN_MODIFIED(ovsrec_port_col_interfaces,
                                           idl_seqno))) {
        /*
         * Check if the admin column changed.
         */

        VLOG_DBG("Port admin state modified\n");
        /* Search for port row which has changed admin_state */
        OVSREC_PORT_FOR_EACH (port_row, idl) {

            /* If the port row is modified then update the
               hw_intf_config for associated interfaces */
            if (OVSREC_IDL_IS_ROW_MODIFIED(port_row, idl_seqno)) {
                /* Go through each interface associated with this port */
                VLOG_DBG("port row which has modified admin state\n");
                /* update our port cache */
                port_data = shash_find_data(&all_ports, port_row->name);
                if (!port_data) {
                    VLOG_DBG("Port cache is NULL\n");
                    continue;
                }

                for (i = 0; i < port_row->n_interfaces; i++)
                {
                    intf_row = port_row->interfaces[i];

                    /* Set the port_admin field to up/down
                       based on port admin state */
                    intf = shash_find_data(&all_interfaces, intf_row->name);
                    if ((port_row->admin == NULL) || (!strcmp(port_row->admin, "up"))) {
                        VLOG_DBG("Set intf->port_admin to up\n");
                        intf->port_admin = PORT_ADMIN_CONFIG_UP;
                    } else {
                        VLOG_DBG("Set intf->port_admin to down\n");
                        intf->port_admin = PORT_ADMIN_CONFIG_DOWN;
                    }
                    intf->user_cfg.admin_state = INTERFACE_USER_CONFIG_ADMIN_DOWN;
                    data = smap_get((const struct smap *)&intf_row->user_config,
                                    INTERFACE_USER_CONFIG_MAP_ADMIN);
                    if (data && (STR_EQ(data, OVSREC_INTERFACE_USER_CONFIG_ADMIN_UP))) {
                        intf->user_cfg.admin_state = INTERFACE_USER_CONFIG_ADMIN_UP;
                        log_event("INTERFACE_UP", EV_KV("interface", intf->name));
                    } else {
                        log_event("INTERFACE_DOWN", EV_KV("interface", intf->name));
                    }
                    set_interface_config(intf_row, intf);
                    rc++;
                }
                rc |= remove_interface_from_port(port_row);
                if (port_data->n_interfaces) {
                    free(port_data->interface);
                }
                port_data->n_interfaces = port_row->n_interfaces;
                if (port_data->n_interfaces) {
                    port_data->interface = xmalloc(port_row->n_interfaces * sizeof(struct ovsrec_interface *));
                }
                VLOG_DBG("port_data->n_interfaces = %zu port_row->n_interfaces = %zu",
                        port_data->n_interfaces, port_row->n_interfaces);
                for (i = 0; i < port_row->n_interfaces; i++) {
                    port_data->interface[i] = port_row->interfaces[i];
                }
            }
        }
    }
    return rc;
}

int
remove_interface_from_port(const struct ovsrec_port *port_row)
{
    int rc = 0, i, j;
    int found;
    const struct ovsrec_interface *intf_row = NULL;
    struct smap hw_cfg_smap;
    struct port_info *port_data;
    struct iface *intf;

    /* Go through each interface associated with this port */
    VLOG_DBG("Add/Delete interface: port row which has modified\n");
    port_data = shash_find_data(&all_ports, port_row->name);
    if (!port_data) {
        VLOG_DBG("port_data is NULL\n");
        return rc;
    }

    if (!port_row->n_interfaces) {
        /* Reset the interface admin state */
        VLOG_DBG("First interface\n");
        VLOG_DBG("deleting interface from port\n");
        for(j = 0; j < port_data->n_interfaces; j++) {
            intf_row = port_data->interface[j];
            intf = shash_find_data(&all_interfaces, intf_row->name);
            if (port_parse_admin(&intf->port_admin, intf_row)) {
                intf->user_cfg.admin_state = intf_parse_admin(intf_row);
                VLOG_INFO("Set the new admin state based on the port state\n");
                set_interface_config(intf_row, intf);
            } else {
                VLOG_DBG("reset interface %s\n", intf_row->name);
                smap_init(&hw_cfg_smap);
                smap_add(&hw_cfg_smap,
                        INTERFACE_HW_INTF_CONFIG_MAP_ENABLE,
                        INTERFACE_HW_INTF_CONFIG_MAP_ENABLE_FALSE);
                ovsrec_interface_set_hw_intf_config(intf_row, &hw_cfg_smap);
                smap_destroy(&hw_cfg_smap);
            }
        }
        rc++;
    }

    for(j = 0; j < port_data->n_interfaces; j++) {
        found = 0;
        for (i = 0; i < port_row->n_interfaces; i++) {
            if((port_data->interface[j] == port_row->interfaces[i])) {
                VLOG_DBG("both have the interfaces = %s\n", port_row->name);
                found = 1;
                break;
            }
        }
        if(!found) {
            /* Reset the inetrface admin state */
            VLOG_DBG("deleting interface from port\n");
            intf_row = port_data->interface[j];
            intf = shash_find_data(&all_interfaces, intf_row->name);
            if (port_parse_admin(&intf->port_admin, intf_row)) {
                VLOG_INFO("Set the new admin state based on the port state\n");
                intf->user_cfg.admin_state = intf_parse_admin(intf_row);
                set_interface_config(intf_row, intf);
            } else {
                VLOG_DBG("reset interface %s\n", intf_row->name);
                smap_init(&hw_cfg_smap);
                smap_add(&hw_cfg_smap,
                        INTERFACE_HW_INTF_CONFIG_MAP_ENABLE,
                        INTERFACE_HW_INTF_CONFIG_MAP_ENABLE_FALSE);
                ovsrec_interface_set_hw_intf_config(intf_row, &hw_cfg_smap);
                smap_destroy(&hw_cfg_smap);
            }
            rc++;

        }
    }
    return rc;
}

static int
port_reconfigure(void)
{
    int rc = 0;
    const struct ovsrec_port *port_row = NULL;
    unsigned int new_idl_seqno = 0;
    struct shash sh_idl_ports;
    struct shash_node *sh_node = NULL, *sh_next = NULL;

    port_row = ovsrec_port_first(idl);

    /* if its not a port related operation then do not go ahead */
    if ( (!OVSREC_IDL_ANY_TABLE_ROWS_MODIFIED(port_row, idl_seqno)) &&
            (!OVSREC_IDL_ANY_TABLE_ROWS_DELETED(port_row, idl_seqno))  &&
            (!OVSREC_IDL_ANY_TABLE_ROWS_INSERTED(port_row, idl_seqno)) )
    {
        VLOG_DBG("Not a port row change\n");
        return rc;
    }

    new_idl_seqno = ovsdb_idl_get_seqno(idl);
    if (new_idl_seqno == idl_seqno) {
        /* There was no change in the dB. */
        return 0;
    }

    /* Collect all the interfaces in the dB. */
    shash_init(&sh_idl_ports);
    OVSREC_PORT_FOR_EACH(port_row, idl) {
        if (!shash_add_once(&sh_idl_ports, port_row->name, port_row)) {
            VLOG_WARN("interface %s specified twice", port_row->name);
        }
    }

    port_row = ovsrec_port_first(idl);
    /* Add new Port. */
    if (OVSREC_IDL_ANY_TABLE_ROWS_INSERTED(port_row, idl_seqno)) {
        SHASH_FOR_EACH(sh_node, &sh_idl_ports) {
            struct port_info *new_port = shash_find_data(&all_ports, sh_node->name);
            if (!new_port) {
                VLOG_DBG("Adding new port %s", sh_node->name);
                add_new_port(sh_node->data);
            }
        }
        /* Delete all interfaces of the deleted port.
         * Use SHASH_FOR_EACH_SAFE since del_old_interface()
         * will delete the current node. */
    } else if (OVSREC_IDL_ANY_TABLE_ROWS_DELETED(port_row, idl_seqno)) {
        SHASH_FOR_EACH_SAFE(sh_node, sh_next, &all_ports) {
            struct ovsrec_port *port = shash_find_data(&sh_idl_ports, sh_node->name);
            if (!port) {
                VLOG_DBG("Deleting Port %s", sh_node->name);
                del_old_port(sh_node);
                rc++;
                goto end;
            }
        }
    }


    /* Number of interfaces/admin state modified. So it could be
       adding more interfaces to port or removing more interfaces from port*/
    rc = add_del_interface_handle_port_config_mods();

end:
    /* Destroy the shash of the IDL interfaces. */
    shash_destroy(&sh_idl_ports);

    return rc;
} /* intfd_reconfigure */

static int
intfd_reconfigure(void)
{
    int rc = 0;
    const struct ovsrec_interface *ifrow = NULL;
    const struct ovsrec_subsystem *subrow = NULL;
    unsigned int new_idl_seqno = 0;
    struct shash sh_idl_interfaces;
    struct shash_node *sh_node = NULL, *sh_next = NULL;

    new_idl_seqno = ovsdb_idl_get_seqno(idl);
    if (new_idl_seqno == idl_seqno) {
        /* There was no change in the dB. */
        return 0;
    }
    VLOG_DBG("Intfd_reconfigure\n");

    /* Need MTU from subsystem table.
     *
     * FIXME: need to add multiple subsystem support
     *
     * For now, hard coding to look for "base" and continue to assume
     * that all interfaces belong to the "base" subsystem.
    */

    base_subsys.mtu = 0;
    OVSREC_SUBSYSTEM_FOR_EACH(subrow, idl) {
        const char *data;
        if (strcmp(subrow->name, "base") == 0) {
            data = smap_get(&subrow->other_info,
                            SUBSYSTEM_OTHER_INFO_MAX_TRANSMISSION_UNIT);
            if (data) {
                base_subsys.mtu = atoi(data);
                if (base_subsys.mtu < INTFD_MIN_ALLOWED_USER_SPECIFIED_MTU) {
                    VLOG_WARN("MTU in hw description file for subsystem %s is "
                              "less than minimum allowed of %d",
                              subrow->name,
                              INTFD_MIN_ALLOWED_USER_SPECIFIED_MTU);
                }
            }
        }
    }

    /* Collect all the interfaces in the dB. */
    shash_init(&sh_idl_interfaces);
    OVSREC_INTERFACE_FOR_EACH(ifrow, idl) {
        if (!shash_add_once(&sh_idl_interfaces, ifrow->name, ifrow)) {
            VLOG_WARN("interface %s specified twice", ifrow->name);
        }
    }

    /* Delete old interfaces.
     * Use SHASH_FOR_EACH_SAFE since del_old_interface()
     * will delete the current node. */
    SHASH_FOR_EACH_SAFE(sh_node, sh_next, &all_interfaces) {
        struct iface *new_intf = shash_find_data(&sh_idl_interfaces, sh_node->name);
        if (!new_intf) {
            VLOG_DBG("Deleting interface %s", sh_node->name);
            del_old_interface(sh_node);
        }
    }

    /* Add new interfaces. */
    SHASH_FOR_EACH(sh_node, &sh_idl_interfaces) {
        struct iface *new_intf = shash_find_data(&all_interfaces, sh_node->name);
        if (!new_intf) {
            VLOG_DBG("Adding new interface %s", sh_node->name);
            add_new_interface(sh_node->data);
        }
    }

    rc = port_reconfigure();
    VLOG_DBG("After port reconfigure rc = %d\n", rc);

    /* Process interface config changes. */
    rc |= handle_interfaces_config_mods(&sh_idl_interfaces);

    /* Update idl_seqno after handling all OVSDB updates. */
    idl_seqno = new_idl_seqno;

    /* Destroy the shash of the IDL interfaces. */
    shash_destroy(&sh_idl_interfaces);

    return rc;
} /* intfd_reconfigure */

static inline bool
intfd_system_is_configured(void)
{
    const struct ovsrec_system *sysrow = NULL;

    if (system_configured) {
        return true;
    }

    sysrow = ovsrec_system_first(idl);

    if (sysrow && sysrow->cur_cfg > INT64_C(0)) {
        VLOG_DBG("System now configured (cur_cfg=%" PRId64 ").",
                 sysrow->cur_cfg);
        return (system_configured = true);
    }

    return false;
} /* intfd_system_is_configured */

void
intfd_run(void)
{
    struct ovsdb_idl_txn *txn;

    /* Process a batch of messages from OVSDB. */
    ovsdb_idl_run(idl);

    if (ovsdb_idl_is_lock_contended(idl)) {
        static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 1);

        VLOG_ERR_RL(&rl, "Another intfd process is running, "
                    "disabling this process until it goes away");

        return;
    } else if (!ovsdb_idl_has_lock(idl)) {
        return;
    }

    /* Nothing to do until system has been configured, i.e., cur_cfg > 0. */
    if (!intfd_system_is_configured()) {
        return;
    }

    /* Update the local configuration and push any changes to the dB. */
    txn = ovsdb_idl_txn_create(idl);
    if (intfd_reconfigure()) {
        VLOG_DBG("Commiting changes\n");
        /* Some OVSDB write needs to happen. */
        ovsdb_idl_txn_commit_block(txn);
    }
    ovsdb_idl_txn_destroy(txn);

    return;
} /* intfd_run */

void
intfd_wait(void)
{
    ovsdb_idl_wait(idl);
} /* intfd_wait */

/** @} end of group intfd */
