/* Interface CLI commands
 *
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 * Copyright (C) 2015-2016 Hewlett Packard Enterprise Development LP
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * File: intf_vty.c
 *
 * Purpose:  To add Interface/Port related configuration and display commands.
 */

#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <pwd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "vtysh/lib/version.h"
#include "getopt.h"
#include "vtysh/command.h"
#include "vtysh/memory.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include "vtysh/vtysh_utils.h"
#include "vswitch-idl.h"
#include "ovsdb-idl.h"
#include "intf_vty.h"
#include "smap.h"
#include "openvswitch/vlog.h"
#include "openswitch-idl.h"
#include "qos_intf.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "vtysh_ovsdb_intf_context.h"
#include "vtysh/utils/vlan_vtysh_utils.h"
#include "vtysh/utils/lacp_vtysh_utils.h"
#include "vtysh/utils/intf_vtysh_utils.h"
#include "openswitch-idl.h"
#include "openswitch-dflt.h"


VLOG_DEFINE_THIS_MODULE(vtysh_interface_cli);
extern struct ovsdb_idl *idl;
extern int
create_sub_interface(const char* subifname);
#define INTF_NAME_SIZE 50
#define IPV6_LENGTH 4

static struct cmd_node interface_node =
   {
      INTERFACE_NODE,
      "%s(config-if)# ",
   };

/* Extract the interface type from interface name */
static char *
extract_intf_type (const char *intf_name)
{
    const char *p = intf_name;
    int count = 0;

    while (*p) {
        if (isdigit (*p)) {
            break;
        }
        else {
            p ++;
            count ++;
        }
    }
    char *intf_type = (char *) malloc(sizeof (char) * (count + 1));
    strncpy (intf_type, intf_name, count);
    intf_type[count] = '\0';

    return intf_type;
}

/* Extract the interface id from the interface-name */
static unsigned long
extract_tag_from_intf_name (const char *intf_name)
{
    unsigned long val = 0;
    const char *p = intf_name;
    while (*p) {
        if (isdigit(*p)) {
             val = val*10 + (*p-'0');
        }
        p++;
   }
   return val;
}

static int
compare_both_proper_intf_names (const char *name_intf1, const char *name_intf2)
{
    char *type_intf1 = extract_intf_type (name_intf1);
    char *type_intf2 = extract_intf_type (name_intf2);

    int str_comparison_result = strcmp (type_intf1, type_intf2);
    free(type_intf1);
    free(type_intf2);
    if (str_comparison_result == 0)
    {
        unsigned long tag_intf1 = extract_tag_from_intf_name (name_intf1);
        unsigned long tag_intf2 = extract_tag_from_intf_name (name_intf2);

        if (tag_intf1 == tag_intf2)
            return 0;
        else if (tag_intf1 < tag_intf2)
            return -1;
        else
            return 1;
     }
     else if (str_comparison_result < 0) {
        return -1;
     }
     else {
        return 1;
     }
}

static int
compare_both_intf_numbers (const char *name_intf1, const char *name_intf2)
{
     uint id_intf1 = 0, id_intf2 = 0;
     uint ext_id_intf1 = 0, ext_id_intf2 = 0;
     unsigned long tag_sub_intf1 = 0, tag_sub_intf2 = 0;

     /* To  handle cases like 10, 54-2, 52-1.1 etc. */
     sscanf(name_intf1, "%d-%d.%lu", &id_intf1, &ext_id_intf1, &tag_sub_intf1);
     sscanf(name_intf2, "%d-%d.%lu", &id_intf2, &ext_id_intf2, &tag_sub_intf2);

     /* To handle cases like 10.1, 54.2 etc. */
     if (strchr (name_intf1, '.') && !strchr (name_intf1, '-')) {
         sscanf(name_intf1, "%d.%lu", &id_intf1, &tag_sub_intf1);
     }

     if (strchr (name_intf2, '.') && !strchr (name_intf2, '-')) {
         sscanf(name_intf2, "%d.%lu", &id_intf2, &tag_sub_intf2);
     }

     if(id_intf1 == id_intf2)
     {
        if(ext_id_intf1 == ext_id_intf2)
        {
           /* For proper positioning of subinterfaces */
           if (tag_sub_intf1 == tag_sub_intf2)
               return 0;
           else if (tag_sub_intf1 < tag_sub_intf2)
               return -1;
           else
               return 1;
        }
        else if(ext_id_intf1 < ext_id_intf2)
           return -1;
        else
           return 1;
     }
     else if (id_intf1 < id_intf2)
           return -1;
     else
           return 1;
}

static int
compare_only_one_proper_intf_name (const char *name1, const char *name2)
{
     if (isdigit(*name1)) {
         return -1;
    }
     else {
         return 1;
    }
}

/* qsort comparator function.
 * Case 1: When both interface name consists of numbers only
 *         Example: 10 and 12-1 etc.
 * Case 2: When one interface name consists of numbers and other is proper name
 *         Example: 10 and vlan20 etc.
 * Case 3: When both interface names are proper names with interface type and id
 *         Example: vlan20 and lo23 etc.
 */
int
compare_nodes_by_interface_name_and_tag (const void *a_, const void *b_)
{
    const struct shash_node *const *a = a_;
    const struct shash_node *const *b = b_;
    char *name_intf1 = (*a)->name;
    char *name_intf2 = (*b)->name;

    /* bridge_normal has to be at the top */
    if (!strcmp (name_intf1, "bridge_normal"))
        return -1;
    else if (!strcmp (name_intf2, "bridge_normal"))
        return 1;

    /* Case 1 */
    if (isdigit(*name_intf1) && isdigit(*name_intf2))
    {
         return compare_both_intf_numbers (name_intf1, name_intf2);
    }
    /* Case 2 */
    else if (isdigit(*name_intf1) || isdigit(*name_intf2))
    {
        return compare_only_one_proper_intf_name (name_intf1, name_intf2);
    }
    /* Case 3 */
    else
    {
        return compare_both_proper_intf_names (name_intf1, name_intf2);
    }

    return 0;
}

/*
 * Sorting function for interface
 * on success, returns sorted interface list.
 */
const struct shash_node **
sort_interface(const struct shash *sh)
{
    if (shash_is_empty(sh)) {
        return NULL;
    } else {
        const struct shash_node **nodes;
        struct shash_node *node;

        size_t i, n;

        n = shash_count(sh);
        nodes = xmalloc(n * sizeof *nodes);
        i = 0;
        SHASH_FOR_EACH (node, sh) {
            nodes[i++] = node;
        }
        ovs_assert(i == n);

        qsort(nodes, n, sizeof *nodes, compare_nodes_by_interface_name_and_tag);
        return nodes;
    }
}


/*
 * CLI "shutdown"
 * default : enabled
 */
DEFUN (cli_intf_shutdown,
        cli_intf_shutdown_cmd,
        "shutdown",
        "Enable/disable an interface\n")
{
    const struct ovsrec_interface * row = NULL;
    const struct ovsrec_port *port_row = NULL;
    struct ovsdb_idl_txn* status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status status;
    struct smap smap_user_config;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    row = ovsrec_interface_first(idl);
    if (!row)
    {
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    OVSREC_INTERFACE_FOR_EACH(row, idl)
    {
        if (strcmp(row->name, (char*)vty->index) == 0)
        {
            smap_clone(&smap_user_config, &row->user_config);

            if (vty_flags & CMD_FLAG_NO_CMD)
            {
                smap_replace(&smap_user_config,
                        INTERFACE_USER_CONFIG_MAP_ADMIN,
                        OVSREC_INTERFACE_USER_CONFIG_ADMIN_UP);
            }
            else
            {
                smap_replace(&smap_user_config,
                        INTERFACE_USER_CONFIG_MAP_ADMIN,
                        OVSREC_INTERFACE_USER_CONFIG_ADMIN_DOWN);
            }
            ovsrec_interface_set_user_config(row, &smap_user_config);
            break;
        }
    }

    OVSREC_PORT_FOR_EACH(port_row, idl)
    {
        if(strcmp(port_row->name, (char*)vty->index) == 0)
        {
            if(vty_flags & CMD_FLAG_NO_CMD)
            {
                ovsrec_port_set_admin(port_row,
                        OVSREC_INTERFACE_ADMIN_STATE_UP);
            }
            else
            {
                ovsrec_port_set_admin(port_row,
                        OVSREC_INTERFACE_ADMIN_STATE_DOWN);
            }
            break;
        }
    }

    status = cli_do_config_finish(status_txn);

    smap_destroy(&smap_user_config);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
    }

    return CMD_OVSDB_FAILURE;
}

DEFUN_NO_FORM (cli_intf_shutdown,
        cli_intf_shutdown_cmd,
        "shutdown",
        "Enable/disable an interface\n");


void
dyncb_helpstr_speeds(struct cmd_token *token, struct vty *vty, \
                     char * const helpstr, int max_strlen)
{
    const struct ovsrec_interface * row = NULL;
    const char *speeds_list = NULL;
    char * tmp = NULL;
    row = ovsrec_interface_first(idl);
    if (!row)
    {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        return;
    }

    OVSREC_INTERFACE_FOR_EACH(row, idl)
    {
        if(strcmp(row->name, vty->index) != 0)
            continue;

        speeds_list = smap_get(&row->hw_intf_info, "speeds");
        if (speeds_list != NULL)
        {
            char * cur_state = calloc(strlen(speeds_list) + 1, sizeof(char));
            strcpy(cur_state, speeds_list);
            tmp = strtok(cur_state, ",");
            while (tmp != NULL)
            {
                if (strcmp(tmp, token->cmd) == 0)
                {
                    snprintf(helpstr, max_strlen, "Mb/s supported");
                    free(cur_state);
                    return;
                }
                tmp = strtok(NULL, ",");
            }
            snprintf(helpstr, max_strlen, "Mb/s not supported");
            free(cur_state);
        }
        else
            snprintf(helpstr, max_strlen, "Mb/s not configured");
    }
    return;
}

/*
 * CLI "speed"
 * default : auto
 * Maximum speed is consitent with maximum speed mentioned in ports.yaml.
 */
DEFUN_DYN_HELPSTR (cli_intf_speed,
      cli_intf_speed_cmd,
      "speed (auto|1000|10000|25000|40000|50000|100000)",
      "Configure the interface speed\n"
      "Auto negotiate speed (Default)\n"
      "1Gb/s\n10Gb/s\n25Gb/s\n40Gb/s\n50Gb/s\n100Gb/s",
      "\n\ndyncb_helpstr_1G\ndyncb_helpstr_10G\ndyncb_helpstr_25G"
      "\ndyncb_helpstr_40G\ndyncb_helpstr_50G\ndyncb_helpstr_100G")
{
    const struct ovsrec_interface * row = NULL;
    struct ovsdb_idl_txn* status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status status;
    struct smap smap_user_config;
    const char *speeds_list = NULL;
    char *tmp = NULL;
    int support_flag = 0;
    char *cur_state = NULL;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    row = ovsrec_interface_first(idl);
    if (!row)
    {
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    OVSREC_INTERFACE_FOR_EACH(row, idl)
    {
        if (strcmp(row->name, (char*)vty->index) == 0)
        {
            if (is_parent_interface_split(row))
            {
                vty_out(vty,
                        "This interface has been split. Operation"
                        " not allowed%s", VTY_NEWLINE);
                cli_do_config_abort (status_txn);
                return CMD_SUCCESS;
            }
            smap_clone(&smap_user_config,&row->user_config);

            if (vty_flags & CMD_FLAG_NO_CMD)
            {
                smap_remove(&smap_user_config,
                        INTERFACE_USER_CONFIG_MAP_SPEEDS);
            }
            else
            {
                if (strcmp(INTERFACE_USER_CONFIG_MAP_SPEEDS_DEFAULT,
                        argv[0]) == 0)
                {
                    smap_remove(&smap_user_config,
                            INTERFACE_USER_CONFIG_MAP_SPEEDS);
                }
                else
                {
                    speeds_list = smap_get(&row->hw_intf_info, "speeds");
                    support_flag = 0;
                    if (speeds_list != NULL)
                    {
                        cur_state = calloc(strlen(speeds_list) + 1,
                                                           sizeof(char));
                        strcpy(cur_state, speeds_list);
                        tmp = strtok(cur_state, ",");
                        while (tmp != NULL)
                        {
                            if (strcmp(tmp, argv[0]) == 0)
                            {
                                support_flag = 1;
                                break;
                            }
                            tmp = strtok(NULL, ",");
                        }
                        free(cur_state);
                        if (support_flag == 0)
                        {
                            vty_out(vty, "Interface doesn't support %s (Mb/s). "
                                        "Supported speed(s) : %s (Mb/s).%s",
                                        argv[0], speeds_list, VTY_NEWLINE);
                            cli_do_config_abort(status_txn);
                            smap_destroy(&smap_user_config);
                            return CMD_SUCCESS;
                         }
                            smap_replace(&smap_user_config,
                                         INTERFACE_USER_CONFIG_MAP_SPEEDS,
                                         argv[0]);
                      }
                }
            }
            ovsrec_interface_set_user_config(row, &smap_user_config);
            break;
        }
    }

    status = cli_do_config_finish(status_txn);

    smap_destroy(&smap_user_config);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
    }

    return CMD_OVSDB_FAILURE;
}

DEFUN_NO_FORM (cli_intf_speed,
        cli_intf_speed_cmd,
        "speed",
        "Enter the interface speed\n");

void
dyncb_helpstr_mtu(struct cmd_token *token, struct vty *vty, \
                  char * const helpstr, int max_strlen)
{
    const struct ovsrec_subsystem * row = NULL;
    const char * mtu = NULL;

    row = ovsrec_subsystem_first(idl);
    if (!row)
    {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        return;
    }

    OVSREC_SUBSYSTEM_FOR_EACH(row, idl)
    {
        mtu = smap_get(&row->other_info, "max_transmission_unit");
        if (mtu != NULL)
            snprintf(helpstr, max_strlen, \
                     "Enter MTU (in bytes) in the range <576-%s>", mtu);
    }
    return;
}

/*
 * CLI "mtu"
 */
DEFUN_DYN_HELPSTR (cli_intf_mtu,
        cli_intf_mtu_cmd,
        "mtu WORD",
        "Configure MTU for the interface\n"
        "Enter MTU (in bytes) in the range <576-9192> (Default: 1500)\n",
        "\n\ndyncb_helpstr_mtu\n")
{
    const struct ovsrec_interface * row = NULL;
    enum ovsdb_idl_txn_status status;
    struct smap smap_user_config;
    struct ovsdb_idl_txn* status_txn;
    long mtu;
    char *endptr;

    /* Validate that MTU is in the range 576-9216 */
    if (!(vty_flags & CMD_FLAG_NO_CMD)) {
        mtu = strtol(argv[0], &endptr, 10);
        if((*endptr != '\0') || (mtu < 576 || mtu > 9192)) {
           vty_out(vty, "Invalid MTU value%s", VTY_NEWLINE);
           return CMD_ERR_NOTHING_TODO;
        }
    }

    status_txn = cli_do_config_start();
    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    row = ovsrec_interface_first(idl);
    if (!row)
    {
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    OVSREC_INTERFACE_FOR_EACH(row, idl)
    {
        if (strcmp(row->name, (char*)vty->index) == 0)
        {
            if (is_parent_interface_split(row))
            {
                vty_out(vty,
                        "This interface has been split. Operation"
                        " not allowed%s", VTY_NEWLINE);
                cli_do_config_abort (status_txn);
                return CMD_SUCCESS;
            }
            smap_clone(&smap_user_config, &row->user_config);

            if (vty_flags & CMD_FLAG_NO_CMD)
            {
                smap_remove(&smap_user_config, INTERFACE_USER_CONFIG_MAP_MTU);
            }
            else
            {
                if (strcmp(INTERFACE_USER_CONFIG_MAP_MTU_DEFAULT,
                        argv[0]) == 0)
                {
                    smap_remove(&smap_user_config,
                            INTERFACE_USER_CONFIG_MAP_MTU);
                }
                else
                {
                    smap_replace(&smap_user_config,
                            INTERFACE_USER_CONFIG_MAP_MTU, argv[0]);
                }
            }
            ovsrec_interface_set_user_config(row, &smap_user_config);
            break;
        }
    }

    status = cli_do_config_finish(status_txn);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED)

        smap_destroy(&smap_user_config);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
    }

    return CMD_OVSDB_FAILURE;
}

DEFUN_NO_FORM (cli_intf_mtu,
        cli_intf_mtu_cmd,
        "mtu",
        "Configure mtu for the interface\n");


/*
 * CLI "duplex"
 * default : full
 */
DEFUN (cli_intf_duplex,
        cli_intf_duplex_cmd,
        "duplex (half|full)",
        "Configure the interface duplex mode\n"
        "Configure half-duplex\n"
        "Configure full-duplex (Default)")
{
    const struct ovsrec_interface * row = NULL;
    struct ovsdb_idl_txn* status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status status;
    struct smap smap_user_config;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    row = ovsrec_interface_first(idl);
    if (!row)
    {
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    OVSREC_INTERFACE_FOR_EACH(row, idl)
    {
        if (strcmp(row->name, (char*)vty->index) == 0)
        {
            if (is_parent_interface_split(row))
            {
                vty_out(vty,
                        "This interface has been split. Operation"
                        " not allowed%s", VTY_NEWLINE);
                cli_do_config_abort (status_txn);
                return CMD_SUCCESS;
            }
            smap_clone(&smap_user_config, &row->user_config);

            if ((vty_flags & CMD_FLAG_NO_CMD)
                    || (strcmp(argv[0], "full") == 0))
            {
                smap_remove(&smap_user_config,
                        INTERFACE_USER_CONFIG_MAP_DUPLEX);
            }
            else
            {
                smap_replace(&smap_user_config,
                        INTERFACE_USER_CONFIG_MAP_DUPLEX,
                        INTERFACE_USER_CONFIG_MAP_DUPLEX_HALF);
            }
            ovsrec_interface_set_user_config(row, &smap_user_config);
            break;
        }
    }

    status = cli_do_config_finish(status_txn);

    smap_destroy(&smap_user_config);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
    }

    return CMD_OVSDB_FAILURE;
}

DEFUN_NO_FORM (cli_intf_duplex,
        cli_intf_duplex_cmd,
        "duplex",
        "Configure the interface duplex mode\n");


/*
 * CLI "flowcontrol"
 * default : off
 */
DEFUN (cli_intf_flowcontrol,
        cli_intf_flowcontrol_cmd,
        "flowcontrol (receive|send) (off|on)",
        "Configure interface flow control\n"
        "Receive pause frames\nSend pause frames\n"
        "Turn off flow-control (Default)\nTurn on flow-control\n")
{
    const struct ovsrec_interface * row = NULL;
    struct ovsdb_idl_txn* status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status status;
    struct smap smap_user_config;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    row = ovsrec_interface_first(idl);
    if (!row)
    {
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    OVSREC_INTERFACE_FOR_EACH(row, idl)
    {
        if (strcmp(row->name, (char*)vty->index) == 0)
        {
            if (is_parent_interface_split(row))
            {
                vty_out(vty,
                        "This interface has been split. Operation"
                        " not allowed%s", VTY_NEWLINE);
                cli_do_config_abort (status_txn);
                return CMD_SUCCESS;
            }
            const char *state_value = smap_get(&row->user_config,
                    INTERFACE_USER_CONFIG_MAP_PAUSE);
            char new_value[INTF_NAME_SIZE] = {0};
            smap_clone(&smap_user_config, &row->user_config);
            if (strcmp(argv[0], "send") == 0)
            {
                if (strcmp(argv[1], "on") == 0)
                {
                    if ((NULL == state_value)
                            || (strcmp(state_value,
                                    INTERFACE_USER_CONFIG_MAP_PAUSE_TX) == 0))
                    {
                        strcpy(new_value, INTERFACE_USER_CONFIG_MAP_PAUSE_TX);
                    }
                    else
                    {
                        strcpy(new_value,
                                INTERFACE_USER_CONFIG_MAP_PAUSE_RXTX);
                    }
                }
                else /* both "flowcontrol send off" and "no flowcontrol send"*/
                {
                    if ((NULL == state_value) ||
                            (strcmp(state_value,
                                    INTERFACE_USER_CONFIG_MAP_PAUSE_TX) == 0))
                    {
                        strcpy(new_value,
                                INTERFACE_USER_CONFIG_MAP_PAUSE_NONE);
                    }
                    else
                    {
                        strcpy(new_value, INTERFACE_USER_CONFIG_MAP_PAUSE_RX);
                    }
                }
            }
            else /* flowcontrol receive */
            {
                if (strcmp(argv[1], "on") == 0)
                {

                    if ((NULL == state_value)
                            || (strcmp(state_value,
                                    INTERFACE_USER_CONFIG_MAP_PAUSE_RX) == 0))
                    {
                        strcpy(new_value, INTERFACE_USER_CONFIG_MAP_PAUSE_RX);
                    }
                    else
                    {
                        strcpy(new_value,
                                INTERFACE_USER_CONFIG_MAP_PAUSE_RXTX);
                    }
                }
                else
                {
                    /* both "flowcontrol receive off"
                       and "no flowcontrol receive" */
                    if ((NULL == state_value) ||
                            (strcmp(state_value,
                                    INTERFACE_USER_CONFIG_MAP_PAUSE_RX) == 0))
                    {
                        strcpy(new_value,
                                INTERFACE_USER_CONFIG_MAP_PAUSE_NONE);
                    }
                    else
                    {
                        strcpy(new_value, INTERFACE_USER_CONFIG_MAP_PAUSE_TX);
                    }
                }
            }

            if (strcmp(new_value, INTERFACE_USER_CONFIG_MAP_PAUSE_NONE) == 0)
            {
                smap_remove(&smap_user_config,
                        INTERFACE_USER_CONFIG_MAP_PAUSE);
            }
            else
            {
                smap_replace(&smap_user_config,
                        INTERFACE_USER_CONFIG_MAP_PAUSE, new_value);
            }
            ovsrec_interface_set_user_config(row, &smap_user_config);
            break;
        }
    }

    status = cli_do_config_finish(status_txn);

    smap_destroy(&smap_user_config);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
    }

    return CMD_OVSDB_FAILURE;
}

DEFUN_NO_FORM (cli_intf_flowcontrol,
        cli_intf_flowcontrol_cmd,
        "flowcontrol (receive|send)",
        "Configure interface flow control\n"
        "Receive pause frames\nSend pause frames\n");



/*
 * CLI "autonegotiation"
 * default : default
 */
DEFUN (cli_intf_autoneg,
        cli_intf_autoneg_cmd,
        "autonegotiation (off|on)",
        "Configure auto-negotiation process for the interface\n"
        "Turn off autonegotiation\nTurn on autonegotiation (Default)\n")
{
    const struct ovsrec_interface * row = NULL;
    struct ovsdb_idl_txn *status_txn = cli_do_config_start();
    enum ovsdb_idl_txn_status status;
    struct smap smap_user_config;

    if (status_txn == NULL)
    {
        VLOG_ERR(OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    row = ovsrec_interface_first(idl);
    if (!row)
    {
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    OVSREC_INTERFACE_FOR_EACH(row, idl)
    {
        if (strcmp(row->name, (char*)vty->index) == 0)
        {
            if (is_parent_interface_split(row))
            {
                vty_out(vty,
                        "This interface has been split. Operation"
                        " not allowed%s", VTY_NEWLINE);
                cli_do_config_abort (status_txn);
                return CMD_SUCCESS;
            }
            smap_clone(&smap_user_config, &row->user_config);

            if (vty_flags & CMD_FLAG_NO_CMD)
            {
                smap_remove(&smap_user_config,
                        INTERFACE_USER_CONFIG_MAP_AUTONEG);
            }
            else
            {
                if (strcmp(INTERFACE_USER_CONFIG_MAP_AUTONEG_DEFAULT,
                        argv[0]) == 0)
                {
                    smap_remove(&smap_user_config,
                            INTERFACE_USER_CONFIG_MAP_AUTONEG);
                }
                else
                {
                    smap_replace(&smap_user_config,
                            INTERFACE_USER_CONFIG_MAP_AUTONEG, argv[0]);
                }
            }
            ovsrec_interface_set_user_config(row, &smap_user_config);
            break;
        }
    }

    status = cli_do_config_finish(status_txn);

    smap_destroy(&smap_user_config);

    if (status == TXN_SUCCESS || status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
    }

    return CMD_OVSDB_FAILURE;
}

DEFUN_NO_FORM (cli_intf_autoneg,
        cli_intf_autoneg_cmd,
        "autonegotiation",
        "Configure autonegotiation process (Default: off)\n");


/*
 * Function : remove_port_reference
 * Responsibility : Remove port reference from VRF / bridge
 *
 * Parameters :
 *   const struct ovsrec_port *port_row: port to be deleted
 */
static void
remove_port_reference (const struct ovsrec_port *port_row)
{
  struct ovsrec_port **ports = NULL;
  const struct ovsrec_vrf *vrf_row = NULL;
  const struct ovsrec_bridge *default_bridge_row = NULL;
  int i,n;

  if (check_port_in_vrf(port_row->name))
    {
      vrf_row = port_match_in_vrf (port_row);
      ports = xmalloc (sizeof *vrf_row->ports * (vrf_row->n_ports - 1));
      for (i = n = 0; i < vrf_row->n_ports; i++)
        {
          if (vrf_row->ports[i] != port_row)
            ports[n++] = vrf_row->ports[i];
        }
      ovsrec_vrf_set_ports (vrf_row, ports, n);
    }

  if (check_port_in_bridge(port_row->name))
    {
      default_bridge_row = ovsrec_bridge_first (idl);
      ports = xmalloc (
          sizeof *default_bridge_row->ports *
          (default_bridge_row->n_ports - 1));
      for (i = n = 0; i < default_bridge_row->n_ports; i++)
        {
          if (default_bridge_row->ports[i] != port_row)
            ports[n++] = default_bridge_row->ports[i];
        }
      ovsrec_bridge_set_ports (default_bridge_row, ports, n);
    }
  free (ports);
}

/*
 * Function : remove_interface_from_lag_port
 * Responsibility : Remove reference to interface in lag port
 * Parameters :
 *   const struct ovsrec_port *port_row: pointer to port row
 *   const struct ovsrec_interface *interface_row: pointer to interface row
 *   bool split: Boolean to identify if parent interface is split or not
 */
void remove_interface_from_lag_port(const struct ovsrec_port *port_row,
                                    const struct ovsrec_interface* interface_row)
{
    struct smap smap = SMAP_INITIALIZER(&smap);
    struct ovsrec_interface **interfaces;
    int i,n = 0;

    /* Remove Aggregation Key */
    smap_clone(&smap, &interface_row->other_config);
    smap_remove(&smap, INTERFACE_OTHER_CONFIG_MAP_LACP_AGGREGATION_KEY);
    ovsrec_interface_set_other_config(interface_row, &smap);
    smap_destroy(&smap);

    /* Unlink the interface from the Port row found*/
    interfaces = xmalloc(sizeof *port_row->interfaces * (port_row->n_interfaces-1));
    for(i = n = 0; i < port_row->n_interfaces; i++)
    {
        if(port_row->interfaces[i] != interface_row)
        {
            interfaces[n++] = port_row->interfaces[i];
        }
    }
    ovsrec_port_set_interfaces(port_row, interfaces, n);
    free(interfaces);
}

/*
 *  Function : port_find_with_references
 *  Responsibility : Remove interface from Port Table, it could
 *  be as a row or a reference inside a port (for example LAG)
 *  Parameters :
 *    const char *if_name : Interface name
 *  Return : void
*/
void
remove_port_and_references(const char *if_name)
{
    const struct ovsrec_port *port_row = NULL;
    const struct ovsrec_interface* intf_row;
    int i = 0;

    OVSREC_PORT_FOR_EACH(port_row, idl)
    {
        if (strcmp(port_row->name, if_name) == 0) {
            remove_port_reference(port_row);
            ovsrec_port_delete(port_row);
        }
        else if (port_row->n_interfaces > 0)
        {
            for (i = 0; i < port_row->n_interfaces; i++) {
                intf_row = port_row->interfaces[i];
                if (strncmp(intf_row->name, if_name, strlen(intf_row->name)) == 0) {
                    remove_interface_from_lag_port(port_row, intf_row);
                }
            }
        }
    }
}

/*
 * Function : handle_port_config
 * Responsibility : Handle deletion of all configuration
 *                  for split/no split cases
 * Parameters :
 *   const struct ovsrec_port *if_row: pointer to interface row
 *   bool split: Boolean to identify if parent interface is split or not
 */
static void
handle_port_config (const struct ovsrec_interface *if_row, bool split)
{
    int i;
    struct smap smap_user_config;
    if (!if_row) {
        VLOG_ERR("Interface row is empty");
        return;
    }
    if (split) {
        remove_port_and_references(if_row->name);
    }
    else {
        for (i = 0; i < if_row->n_split_children; i++) {
            remove_port_and_references(if_row->split_children[i]->name);
            smap_clone(&smap_user_config, &if_row->split_children[i]->user_config);
            smap_replace(&smap_user_config,INTERFACE_USER_CONFIG_MAP_ADMIN,
                OVSREC_INTERFACE_USER_CONFIG_ADMIN_DOWN);
            ovsrec_interface_set_user_config(if_row->split_children[i], &smap_user_config);
        }
    }
}

/*
 * CLI "split"
 * default : no split
 */
DEFUN (cli_intf_split,
        cli_intf_split_cmd,
        "split",
        "Configure QSFP interface for 4x 10Gb operation "
        "or 1x 40Gb operation.\n")
{
  const struct ovsrec_interface * row = NULL;
  struct ovsdb_idl_txn *status_txn = cli_do_config_start();
  enum ovsdb_idl_txn_status status;
  char flag = '0';
  bool proceed = false;

  if (status_txn == NULL)
    {
      VLOG_ERR (OVSDB_TXN_CREATE_ERROR);
      cli_do_config_abort (status_txn);
      return CMD_OVSDB_FAILURE;
    }

  row = ovsrec_interface_first (idl);
  if (!row)
    {
      cli_do_config_abort (status_txn);
      return CMD_OVSDB_FAILURE;
    }

  OVSREC_INTERFACE_FOR_EACH(row, idl)
    {
      if (strcmp(row->name, (char*)vty->index) == 0)
        {
          /* if not splittable, warn */
          const char *split_value = NULL;
          struct smap smap_user_config;

          smap_clone(&smap_user_config,&row->user_config);

          split_value = smap_get(&row->hw_intf_info,
                  INTERFACE_HW_INTF_INFO_MAP_SPLIT_4);
          if ((split_value == NULL) ||
                  (strcmp(split_value,
                          INTERFACE_HW_INTF_INFO_MAP_SPLIT_4_TRUE) != 0))
            {
               vty_out(vty, "Warning: split operation only applies to"
                       " QSFP interfaces with split capability%s",
                       VTY_NEWLINE);
            }
          if (vty_flags & CMD_FLAG_NO_CMD)
            {
              smap_remove(&smap_user_config,
                      INTERFACE_USER_CONFIG_MAP_LANE_SPLIT);
            }
          else
            {
              smap_replace(&smap_user_config,
                      INTERFACE_USER_CONFIG_MAP_LANE_SPLIT,
                      INTERFACE_USER_CONFIG_MAP_LANE_SPLIT_SPLIT);
            }
          ovsrec_interface_set_user_config (row, &smap_user_config);
          /* Reconfiguration should only be done when split/no split commands
           * are entered on split parent interfaces */
          if ((split_value != NULL) &&
              (strcmp(split_value,
                      INTERFACE_HW_INTF_INFO_MAP_SPLIT_4_TRUE) == 0))
            {
              if (vty_flags & CMD_FLAG_NO_CMD)
                {
                  vty_out (vty, "Warning: This will remove all L2/L3 configuration"
                           " on child interfaces.\nDo you want to continue [y/n]? ");
                }
              else
                {
                  vty_out (vty, "Warning: This will remove all L2/L3 configuration"
                           " on parent interface.\nDo you want to continue [y/n]? ");
                }
              while (1)
                {
                  scanf (" %c", &flag);
                  if (flag == 'y' || flag == 'Y')
                    {
                      handle_port_config (row,
                                          (vty_flags & CMD_FLAG_NO_CMD) ? false : true);
                      proceed = true;
                      break;
                    }
                  else if (flag == 'n' || flag == 'N')
                    {
                      vty_out (vty,"%s",VTY_NEWLINE);
                      proceed = false;
                      break;
                    }
                  else
                    {
                      vty_out (vty, "\r                                  ");
                      vty_out (vty, "\rDo you wish to continue [y/n]? ");
                    }
                }
            }
          smap_destroy(&smap_user_config);
          break;
        }
    }

  if (!proceed)
    {
      cli_do_config_abort (status_txn);
      return CMD_SUCCESS;
    }

  status = cli_do_config_finish (status_txn);

  if (status == TXN_SUCCESS || status == TXN_UNCHANGED)
    {
      return CMD_SUCCESS;
    }
  else
    {
      VLOG_ERR (OVSDB_TXN_COMMIT_ERROR);
    }

  return CMD_OVSDB_FAILURE;
}

DEFUN_NO_FORM (cli_intf_split,
        cli_intf_split_cmd,
        "split",
        "Configure QSFP interface for 4x 10Gb "
        "operation or 1x 40Gb operation\n");

#define PRINT_INT_HEADER_IN_SHOW_RUN if (!bPrinted) \
{ \
bPrinted = true;\
vty_out (vty, "interface %s %s", row->name, VTY_NEWLINE);\
}

/*
 *  Function : parse_vlan
 *  Responsibility : Used for VLAN related config
 *  Parameters :
 *      const char *if_name           : Name of interface
 *      struct vty* vty               : Used for ouput
 */
int
parse_vlan(const char *if_name, struct vty* vty)
{
    const struct ovsrec_port *port_row;
    int i;

    port_row = port_find(if_name);
    if (port_row == NULL)
    {
        return 0;
    }

    if (port_row->vlan_mode == NULL)
    {
        return 0;
    }
    else if (strcmp(port_row->vlan_mode, OVSREC_PORT_VLAN_MODE_ACCESS) == 0)
    {
        vty_out(vty, "%3s%s%ld%s", "", "vlan access ",
                (int64_t)ops_port_get_tag(port_row), VTY_NEWLINE);
    }
    else if (strcmp(port_row->vlan_mode, OVSREC_PORT_VLAN_MODE_TRUNK) == 0)
    {
        for (i = 0; i < port_row->n_vlan_trunks; i++)
        {
            vty_out(vty, "%3s%s%ld%s", "", "vlan trunk allowed ",
                    (int64_t)ops_port_get_trunks(port_row, i), VTY_NEWLINE);
        }
    }
    else if (strcmp(port_row->vlan_mode,
            OVSREC_PORT_VLAN_MODE_NATIVE_UNTAGGED) == 0)
    {
        if (port_row->vlan_tag != NULL)
        {
            vty_out(vty, "%3s%s%ld%s", "", "vlan trunk native ",
                    (int64_t)ops_port_get_tag(port_row), VTY_NEWLINE);
        }
        for (i = 0; i < port_row->n_vlan_trunks; i++)
        {
            vty_out(vty, "%3s%s%ld%s", "", "vlan trunk allowed ",
                    (int64_t)ops_port_get_trunks(port_row, i), VTY_NEWLINE);
        }
    }
    else if (strcmp(port_row->vlan_mode,
            OVSREC_PORT_VLAN_MODE_NATIVE_TAGGED) == 0)
    {
        if (port_row->vlan_tag != NULL)
        {
            vty_out(vty, "%3s%s%ld%s", "", "vlan trunk native ",
                    (int64_t)ops_port_get_tag(port_row), VTY_NEWLINE);
        }
        vty_out(vty, "%3s%s%s", "", "vlan trunk native tag",VTY_NEWLINE);
        for (i = 0; i < port_row->n_vlan_trunks; i++)
        {
            vty_out(vty, "%3s%s%ld%s", "", "vlan trunk allowed ",
                    (int64_t)ops_port_get_trunks(port_row, i), VTY_NEWLINE);
        }
    }

    return 0;
}

/*
 *  Function : parse_l3config
 *  Responsibility : Used for L3 related config
 *  Parameters :
 *      const char *if_name           : Name of interface
 *      struct vty* vty               : Used for ouput
 */
static int
parse_l3config(const char *if_name, struct vty *vty)
{
    const struct ovsrec_port *port_row;
    const struct ovsrec_vrf *vrf_row;
    size_t i;

    port_row = port_find(if_name);
    if (!port_row) {
        return 0;
    }
    if (!check_iface_in_vrf(if_name)) {
        vty_out(vty, "%3s%s%s", "", "no routing", VTY_NEWLINE);
        parse_vlan(if_name, vty);
    }
    if (check_iface_in_vrf(if_name)) {
        vrf_row = port_match_in_vrf(port_row);
        if (display_l3_info(port_row, vrf_row)) {
            if (strcmp(vrf_row->name, DEFAULT_VRF_NAME) != 0) {
                vty_out(vty, "%3s%s%s%s", "", "vrf attach ", vrf_row->name,
                        VTY_NEWLINE);
            }
            if (port_row->ip4_address) {
                vty_out(vty, "%3s%s%s%s", "", "ip address ",
                        port_row->ip4_address, VTY_NEWLINE);
            }
            for (i = 0; i < port_row->n_ip4_address_secondary; i++) {
                vty_out(vty, "%3s%s%s%s%s", "", "ip address ",
                        port_row->ip4_address_secondary[i], " secondary",
                        VTY_NEWLINE);
            }
            if (port_row->ip6_address) {
                vty_out(vty, "%3s%s%s%s", "", "ipv6 address ",
                        port_row->ip6_address, VTY_NEWLINE);
            }
            for (i = 0; i < port_row->n_ip6_address_secondary; i++) {
                vty_out(vty, "%3s%s%s%s%s", "", "ipv6 address ",
                        port_row->ip6_address_secondary[i], " secondary",
                        VTY_NEWLINE);
            }
            if (smap_get(&port_row->other_config,
                         PORT_OTHER_CONFIG_MAP_PROXY_ARP_ENABLED)) {
                vty_out(vty, "%3s%s%s", "", "ip proxy-arp", VTY_NEWLINE);
            }
            if (smap_get(&port_row->other_config,
                         PORT_OTHER_CONFIG_MAP_LOCAL_PROXY_ARP_ENABLED)) {
                vty_out(vty, "%3s%s%s", "", "ip local-proxy-arp", VTY_NEWLINE);
            }
        }
    }
    return 0;
}

/*Function to get the intervals from port table. */
int64_t
ospf_get_port_intervals(const struct ovsrec_port* port_row,
                             const char *key)
{
    int i = 0;

    if (!port_row || !key)
        return 0;

    for (i = 0; i < port_row->n_ospf_intervals; i++)
    {
        if (!strcmp(port_row->key_ospf_intervals[i], key))
            return port_row->value_ospf_intervals[i];
    }

    return 0;
}

static int
print_interface_ospf(const char *if_name, struct vty *vty, bool *bPrinted)
{
    const struct ovsrec_port *port_row = NULL;
    const struct ovsrec_ospf_interface *ospf_interface_row = NULL;
    int i = 0;
    int64_t interval = 0;
    int64_t int_val = 0;

    /* Get the interface row for the interface name passed. */
    OVSREC_OSPF_INTERFACE_FOR_EACH(ospf_interface_row, idl)
    {
        if (strcmp(ospf_interface_row->name, if_name) == 0)
            break;
    }

    if (ospf_interface_row == NULL)
    {
        return -1;
    }
    else
    {
        port_row = ospf_interface_row->port;
    }

    if (port_row == NULL)
    {
        return -1;
    }

    if (!(*bPrinted))
    {
        *bPrinted = true;
        vty_out (vty, "interface %s %s", if_name, VTY_NEWLINE);
    }

    interval = ospf_get_port_intervals(port_row,
                                       OSPF_KEY_HELLO_INTERVAL);
    if ((interval > 0) && (interval != OSPF_HELLO_INTERVAL_DEFAULT))
        vty_out(vty, "%4s%s %ld%s", " ",
                           "ip ospf hello-interval", interval, VTY_NEWLINE);

    interval = ospf_get_port_intervals(port_row, OSPF_KEY_DEAD_INTERVAL);
    if ((interval > 0) && (interval != OSPF_DEAD_INTERVAL_DEFAULT))
        vty_out(vty, "%4s%s %ld%s", " ",
                              "ip ospf dead-interval", interval, VTY_NEWLINE);

    interval = ospf_get_port_intervals(port_row,
                                       OSPF_KEY_RETRANSMIT_INTERVAL);
    if ((interval > 0) && (interval != OSPF_RETRANSMIT_INTERVAL_DEFAULT))
        vty_out(vty, "%4s%s %ld%s", " ", "ip ospf retransmit-interval",
                interval, VTY_NEWLINE);

    interval = ospf_get_port_intervals(port_row, OSPF_KEY_TRANSMIT_DELAY);
    if ((interval > 0) && (interval != OSPF_TRANSMIT_DELAY_DEFAULT))
        vty_out(vty, "%4s%s %ld%s", " ",
                              "ip ospf transmit-delay", interval, VTY_NEWLINE);

    if (port_row->ospf_priority &&
        (*port_row->ospf_priority != 1))
    {
        int_val = *port_row->ospf_priority;
        vty_out(vty, "%4s%s %ld%s", " ",
                              "ip ospf priority", int_val, VTY_NEWLINE);
    }

    if ((port_row->n_ospf_mtu_ignore > 0) &&
        (*port_row->ospf_mtu_ignore == true))
        vty_out(vty, "%4s%s%s", " ",
                              "ip ospf mtu-ignore", VTY_NEWLINE);

    if (port_row->ospf_if_out_cost &&
        (*port_row->ospf_if_out_cost != OSPF_DEFAULT_COST))
    {
        int_val = *port_row->ospf_if_out_cost;
        vty_out(vty, "%4s%s %ld%s", " ",
                              "ip ospf cost", int_val, VTY_NEWLINE);
    }

    if ((port_row->ospf_if_type) &&
        (strcmp(port_row->ospf_if_type,
                OVSREC_PORT_OSPF_IF_TYPE_OSPF_IFTYPE_POINTOPOINT) == 0))
        vty_out(vty, "%4s%s %s%s", " ",
                              "ip ospf network", "point-to-point", VTY_NEWLINE);


    if (port_row->ospf_auth_type)
    {
        if (!strcmp(port_row->ospf_auth_type, OVSREC_PORT_OSPF_AUTH_TYPE_TEXT))
            vty_out(vty, "%4s%s%s", " ", "ip ospf authentication", VTY_NEWLINE);
        else if (!strcmp(port_row->ospf_auth_type,
                         OVSREC_PORT_OSPF_AUTH_TYPE_MD5))
            vty_out(vty, "%4s%s%s", " ",
                         "ip ospf authentication message-digest", VTY_NEWLINE);
        else if (!strcmp(port_row->ospf_auth_type,
                         OVSREC_PORT_OSPF_AUTH_TYPE_NULL))
            vty_out(vty, "%4s%s%s", " ",
                                  "ip ospf authentication null", VTY_NEWLINE);
    }

    for (i = 0; i < port_row->n_ospf_auth_md5_keys; i++)
    {
        vty_out(vty, "%4sip ospf message-digest-key %ld md5 %s%s", " ",
                              port_row->key_ospf_auth_md5_keys[i],
                              port_row->value_ospf_auth_md5_keys[i],
                              VTY_NEWLINE);

    }

    if (port_row->ospf_auth_text_key)
        vty_out(vty, "%4s%s %s%s", " ",
                              "ip ospf authentication-key",
                              port_row->ospf_auth_text_key, VTY_NEWLINE);

    return 0;
}

static int
print_interface_lag(const char *if_name, struct vty *vty, bool *bPrinted)
{
    const struct ovsrec_port *port_row = NULL;
    const struct ovsrec_interface *if_row = NULL;
    int k=0;

    OVSREC_PORT_FOR_EACH(port_row, idl)
    {
        if (strncmp(port_row->name, LAG_PORT_NAME_PREFIX, LAG_PORT_NAME_PREFIX_LENGTH) == 0)
        {
            for (k = 0; k < port_row->n_interfaces; k++)
            {
                if_row = port_row->interfaces[k];
                if(strncmp(if_name, if_row->name, MAX_IFNAME_LENGTH) == 0)
                {
                    if (!(*bPrinted))
                    {
                        *bPrinted = true;
                        vty_out (vty, "interface %s %s", if_name, VTY_NEWLINE);
                    }
                    vty_out(vty, "%3s%s %s%s", "", "lag",
                            &port_row->name[LAG_PORT_NAME_PREFIX_LENGTH], VTY_NEWLINE);
                }
            }
        }
    }
    return 0;
}

static int
parse_lacp_othercfg(const struct smap *ifrow_config, const char *if_name,
                                                struct vty *vty, bool *bPrinted)
{
    const char *data = NULL;

    data = smap_get(ifrow_config, INTERFACE_OTHER_CONFIG_MAP_LACP_PORT_ID);

    if (data)
    {
        if (!(*bPrinted))
        {
            *bPrinted = true;
            vty_out (vty, "interface %s%s", if_name, VTY_NEWLINE);
        }

        vty_out (vty, "%3s%s %s%s", "", "lacp port-id", data, VTY_NEWLINE);
    }

    data = smap_get(ifrow_config, INTERFACE_OTHER_CONFIG_MAP_LACP_PORT_PRIORITY);

    if (data)
    {
        if (!(*bPrinted))
        {
            *bPrinted = true;
            vty_out (vty, "interface %s%s", if_name, VTY_NEWLINE);
        }

            vty_out (vty, "%3s%s %s%s", "", "lacp port-priority", data, VTY_NEWLINE);

    }

    return 0;
}

static int
parse_lag(struct vty *vty, int argc, const char *argv[])
{
    const char *data = NULL;
    const struct ovsrec_port *port_row = NULL;
    bool one_lag_to_show;

    // Return if argv is not a LAG port
    if (argc != 0 &&
        strncmp(argv[0], LAG_PORT_NAME_PREFIX, LAG_PORT_NAME_PREFIX_LENGTH) != 0) {
        return 0;
    }

    OVSREC_PORT_FOR_EACH(port_row, idl) {
        if (strncmp(port_row->name, LAG_PORT_NAME_PREFIX, LAG_PORT_NAME_PREFIX_LENGTH) == 0) {
            one_lag_to_show = true;
            if (argc != 0) {
                if (strncmp(port_row->name, argv[0], strlen(argv[0])) != 0) {
                    one_lag_to_show = false;
                }
            }
            if (one_lag_to_show) {
               /* Print the LAG port name because lag port is present. */
               vty_out (vty, "interface lag %s%s",
                        &port_row->name[LAG_PORT_NAME_PREFIX_LENGTH], VTY_NEWLINE);

               data = port_row->admin;
               if (data && strncmp(data, OVSREC_PORT_ADMIN_UP,
                   strlen(OVSREC_PORT_ADMIN_UP)) == 0) {
                   vty_out(vty, "%3s%s%s", "", "no shutdown", VTY_NEWLINE);
               }

               if (check_port_in_bridge(port_row->name)) {
                   vty_out (vty, "%3s%s%s", "", "no routing", VTY_NEWLINE);
                   parse_vlan(port_row->name, vty);
               }

               data = port_row->lacp;
               if (data && strcmp(data, OVSREC_PORT_LACP_OFF) != 0) {
                   vty_out (vty, "%3slacp mode %s%s"," ",data, VTY_NEWLINE);
               }

               data = smap_get(&port_row->other_config, "bond_mode");

               if (data) {
                  vty_out (vty, "%3shash %s%s"," ",data, VTY_NEWLINE);
               }

               data = smap_get(&port_row->other_config, "lacp-fallback-ab");

               if (data) {
                  if (VTYSH_STR_EQ(data, "true")) {
                      vty_out (vty, "%3slacp fallback%s"," ", VTY_NEWLINE);
                  }
               }

               data = NULL;
               data = smap_get(&port_row->other_config,
                               PORT_OTHER_CONFIG_MAP_LACP_FALLBACK_MODE);
               if (data) {
                   if (VTYSH_STR_EQ(data,
                                    PORT_OTHER_CONFIG_LACP_FALLBACK_MODE_ALL_ACTIVE)) {
                       vty_out (vty, "%3slacp fallback mode all_active%s",
                                " ", VTY_NEWLINE);
                   }
               }

               data = NULL;
               data = smap_get(&port_row->other_config,
                               PORT_OTHER_CONFIG_MAP_LACP_FALLBACK_TIMEOUT);
               if (data) {
                   vty_out (vty, "%3slacp fallback timeout %s%s"," ",
                            data, VTY_NEWLINE);
               }

               data = smap_get(&port_row->other_config, "lacp-time");

               if (data) {
                  vty_out (vty, "%3slacp rate %s%s"," ", data, VTY_NEWLINE);
               }

               bool bPrinted = true;
               qos_trust_port_show_running_config(port_row, &bPrinted, "interface");
               qos_apply_port_show_running_config(port_row, &bPrinted, "interface");
               qos_cos_port_show_running_config(port_row, &bPrinted, "interface");
               qos_dscp_port_show_running_config(port_row, &bPrinted, "interface");
            }

            if(port_row->ip4_address)
            {
                vty_out(vty, "%3sip address %s %s", "", port_row->ip4_address, VTY_NEWLINE);
            }

            if(port_row->ip6_address)
            {
                vty_out(vty, "%3sipv6 address %s %s", "", port_row->ip6_address, VTY_NEWLINE);
            }

        }
    }

    return 0;
}

/* Given a port name, display 'no sflow enable' if it is disabled in CLI on
 * an interface. */
void
show_sflow_config(const char *name, const char *align, bool show_cmd)
{
    const struct ovsrec_port *port_row = NULL;
    struct smap other_config = SMAP_INITIALIZER(&other_config);
    const char *value;

    if (name == NULL) {
        VLOG_ERR("Null interface name passed. Can't display sflow info on it.");
        return;
    }

    port_row = port_find(name);
    if (port_row == NULL) {
        VLOG_DBG("No port entry found for %s. Can't display sflow info on it.", name);
        return;
    }

    smap_clone(&other_config, &port_row->other_config);

    if ((value = smap_get(&other_config,
                          PORT_OTHER_CONFIG_SFLOW_PER_INTERFACE_KEY_STR))) {
        if (strcmp(value,
                   PORT_OTHER_CONFIG_SFLOW_PER_INTERFACE_VALUE_FALSE) == 0) {
            if (show_cmd) {
                vty_out(vty, "%ssFlow is disabled%s", align, VTY_NEWLINE);
            } else {
                vty_out(vty, "%sno sflow enable%s", align, VTY_NEWLINE);
            }
        }
    }

    smap_destroy(&other_config);
}

static int
cli_show_run_interface_exec (struct cmd_element *self, struct vty *vty,
        int flags, int argc, const char *argv[])
{
    const struct ovsrec_interface *row = NULL;
    const struct ovsrec_dhcp_relay *row_serv = NULL;
    const struct ovsrec_udp_bcast_forwarder_server *udp_row_serv = NULL;
    const struct ovsdb_datum *datum = NULL;
    const char *cur_state =NULL;
    struct shash sorted_interfaces;
    bool bPrinted = false;
    size_t i = 0;
    int udp_dport = 0;
    char *buff = NULL, *serverip = NULL;
    const struct shash_node **nodes;
    int idx, count;

    shash_init(&sorted_interfaces);

    OVSREC_INTERFACE_FOR_EACH(row, idl) {
       shash_add(&sorted_interfaces, row->name, (void *)row);
    }

    nodes = sort_interface(&sorted_interfaces);
    count = shash_count(&sorted_interfaces);

    for (idx = 0; idx < count; idx++) {

        row = (const struct ovsrec_interface *)nodes[idx]->data;

        if (0 != argc) {
            if ((NULL != argv[0]) && (0 != strcmp(argv[0], row->name))) {
                continue;
            }
        }
        bPrinted = false;
        cur_state = smap_get(&row->user_config,
                INTERFACE_USER_CONFIG_MAP_ADMIN);
        if ((NULL != cur_state)
                && (strcmp(cur_state,
                        OVSREC_INTERFACE_USER_CONFIG_ADMIN_UP) == 0))
        {
            PRINT_INT_HEADER_IN_SHOW_RUN;
            vty_out(vty, "   no shutdown %s", VTY_NEWLINE);
        }

        if(row->n_subintf_parent)
        {
           int vlan_tag = row->key_subintf_parent[0];
           if (0 != vlan_tag)
           {
               PRINT_INT_HEADER_IN_SHOW_RUN;
               vty_out(vty, "   encapsulation dot1Q %d %s", vlan_tag, VTY_NEWLINE);
           }
        }

        cur_state = smap_get(&row->user_config,
                INTERFACE_USER_CONFIG_MAP_SPEEDS);
        if ((NULL != cur_state)
                && (strcmp(cur_state,
                        INTERFACE_USER_CONFIG_MAP_SPEEDS_DEFAULT) != 0))
        {
            PRINT_INT_HEADER_IN_SHOW_RUN;
            vty_out(vty, "   speed %s %s", cur_state, VTY_NEWLINE);
        }

        cur_state = smap_get(&row->user_config,
                INTERFACE_USER_CONFIG_MAP_MTU);
        if ((NULL != cur_state)
                && (strcmp(cur_state,
                        INTERFACE_USER_CONFIG_MAP_MTU_DEFAULT) != 0))
        {
            PRINT_INT_HEADER_IN_SHOW_RUN;
            vty_out(vty, "   mtu %s %s", cur_state, VTY_NEWLINE);
        }

        cur_state = smap_get(&row->user_config,
                INTERFACE_USER_CONFIG_MAP_DUPLEX);
        if ((NULL != cur_state)
                && (strcmp(cur_state,
                        INTERFACE_USER_CONFIG_MAP_DUPLEX_FULL) != 0))
        {
            PRINT_INT_HEADER_IN_SHOW_RUN;
            vty_out(vty, "   duplex %s %s", cur_state, VTY_NEWLINE);
        }

        const struct ovsrec_port* port_row = port_find(row->name);
        qos_trust_port_show_running_config(port_row, &bPrinted, "interface");
        qos_apply_port_show_running_config(port_row, &bPrinted, "interface");
        qos_cos_port_show_running_config(port_row, &bPrinted, "interface");
        qos_dscp_port_show_running_config(port_row, &bPrinted, "interface");

        cur_state = smap_get(&row->user_config,
                INTERFACE_USER_CONFIG_MAP_PAUSE);
        if ((NULL != cur_state)
                && (strcmp(cur_state,
                        INTERFACE_USER_CONFIG_MAP_PAUSE_NONE) != 0))
        {
            PRINT_INT_HEADER_IN_SHOW_RUN;
            if (strcmp(cur_state, INTERFACE_USER_CONFIG_MAP_PAUSE_RX) == 0)
            {
                vty_out(vty, "   flowcontrol receive on %s", VTY_NEWLINE);
            }
            else if (strcmp(cur_state,
                    INTERFACE_USER_CONFIG_MAP_PAUSE_TX) == 0)
            {
                vty_out(vty, "   flowcontrol send on %s", VTY_NEWLINE);
            }
            else
            {
                vty_out(vty, "   flowcontrol receive on %s", VTY_NEWLINE);
                vty_out(vty, "   flowcontrol send on %s", VTY_NEWLINE);
            }

            if (0 != argc)  /* filter applied, break - as data printed */
            {
                break;
            }
        }

        cur_state = smap_get(&row->user_config,
                INTERFACE_USER_CONFIG_MAP_AUTONEG);
        if ((NULL != cur_state)
                && (strcmp(cur_state,
                        INTERFACE_USER_CONFIG_MAP_AUTONEG_DEFAULT) != 0)
                && (strcmp(cur_state,
                        INTERFACE_USER_CONFIG_MAP_AUTONEG_ON) != 0))
        {
            PRINT_INT_HEADER_IN_SHOW_RUN;
            vty_out(vty, "   autonegotiation %s %s", cur_state, VTY_NEWLINE);
        }

        cur_state = smap_get(&row->user_config,
                INTERFACE_USER_CONFIG_MAP_LANE_SPLIT);
        if ((NULL != cur_state)
                && (strcmp(cur_state,
                        INTERFACE_USER_CONFIG_MAP_LANE_SPLIT_SPLIT) == 0))
        {
            PRINT_INT_HEADER_IN_SHOW_RUN;
            vty_out(vty, "   split %s", VTY_NEWLINE);
        }

        if (port_find(row->name))
        {
            PRINT_INT_HEADER_IN_SHOW_RUN;
        }

        /* show sFlow config, if present */
        show_sflow_config(row->name, "   ", false);

        parse_l3config(row->name, vty);

        parse_lacp_othercfg(&row->other_config, row->name, vty, &bPrinted);

        print_interface_lag(row->name, vty, &bPrinted);

        /*
         * Displaying the dhcp-relay helper addresses
         * and bootp-gateway addresses
         */
        OVSREC_DHCP_RELAY_FOR_EACH (row_serv, idl)
        {
            /* get the interface details. */
            if(row_serv->port)
            {
                if (!strcmp(row_serv->port->name, row->name))
                {
                    for (i = 0; i < row_serv->n_ipv4_ucast_server; i++)
                    {
                        buff = row_serv->ipv4_ucast_server[i];
                        vty_out(vty, "   ip helper-address %s%s",
                                     buff, VTY_NEWLINE);
                    }
                    buff = (char *)smap_get(&row_serv->other_config,
                                DHCP_RELAY_OTHER_CONFIG_MAP_BOOTP_GATEWAY);
                    if (buff)
                    {
                        vty_out(vty, "   ip bootp-gateway %s%s",
                                     buff, VTY_NEWLINE);
                    }
                }
            }
        }

        /* Displaying the UDP forward-protocol addresses */
        OVSREC_UDP_BCAST_FORWARDER_SERVER_FOR_EACH (udp_row_serv, idl)
        {
            if (udp_row_serv->src_port)
            {
                if (!strcmp(udp_row_serv->src_port->name, row->name))
                {
                    for (i = 0; i < udp_row_serv->n_ipv4_ucast_server; i++)
                    {
                        serverip = udp_row_serv->ipv4_ucast_server[i];
                        datum =
                            ovsrec_udp_bcast_forwarder_server_get_udp_dport
                                    (udp_row_serv, OVSDB_TYPE_INTEGER);
                        if ((NULL != datum) && (datum->n > 0))
                        {
                            udp_dport = datum->keys[0].integer;
                        }
                        /* UDP Broadcast Forwarder information. */
                        vty_out(vty, "%3s%s %s %d%s", "",
                            "ip forward-protocol udp", serverip, udp_dport,
                            VTY_NEWLINE);
                    }
                }
            }
        }

        print_interface_ospf(row->name, vty, &bPrinted);

        if (bPrinted) {
            vty_out(vty, "   exit%s", VTY_NEWLINE);
            if (0 != argc) {
               return CMD_SUCCESS;
            }

        }
    }

    shash_destroy(&sorted_interfaces);
    free(nodes);

    parse_lag(vty, argc, argv);

    return CMD_SUCCESS;
}

DEFUN (cli_intf_show_run_intf,
        cli_intf_show_run_intf_cmd,
        "show running-config interface",
        SHOW_STR
        "Current running configuration\n"
        INTERFACE_STR)
{
    return cli_show_run_interface_exec (self, vty, vty_flags, 0, 0);
}

DEFUN (cli_intf_show_run_intf_if,
        cli_intf_show_run_intf_if_cmd,
        "show running-config interface IFNAME",
        SHOW_STR
        "Current running configuration\n"
        INTERFACE_STR
        IFNAME_STR)
{
    return cli_show_run_interface_exec (self, vty, vty_flags, 1, argv);
}

int
cli_show_run_interface_mgmt_exec (struct cmd_element *self, struct vty *vty)
{
    const struct ovsrec_system *vswrow;
    const char *data = NULL;
    const char *ip = NULL;
    const char *subnet = NULL;
    const char *dns_1 = NULL;
    const char *dns_2 = NULL;

    vswrow = ovsrec_system_first(idl);
    if (!vswrow)
    {
        VLOG_ERR(OVSDB_ROW_FETCH_ERROR);
        return CMD_OVSDB_FAILURE;
    }

    data = smap_get(&vswrow->mgmt_intf,SYSTEM_MGMT_INTF_MAP_MODE);
    if (!data)
    {
        /* If not present then mode is dhcp.
           So nothing to display since dhcp is the default. */
        return e_vtysh_ok;
    }

    if (VTYSH_STR_EQ(data, SYSTEM_MGMT_INTF_MAP_MODE_STATIC))
    {
        vty_out(vty, "%s%s", "interface mgmt", VTY_NEWLINE);
        ip = smap_get(&vswrow->mgmt_intf,SYSTEM_MGMT_INTF_MAP_IP);
        subnet = smap_get(&vswrow->mgmt_intf,SYSTEM_MGMT_INTF_MAP_SUBNET_MASK);
        if (ip && subnet && (strcmp(ip,MGMT_INTF_DEFAULT_IP) != 0) )
            vty_out(vty, "%4sip static %s/%s%s","",ip,subnet, VTY_NEWLINE);

        ip = smap_get(&vswrow->mgmt_intf,SYSTEM_MGMT_INTF_MAP_IPV6);
        if (ip && (strcmp(ip,MGMT_INTF_DEFAULT_IPV6) != 0))
        {
            vty_out(vty, "%4sip static %s %s%s","",ip,subnet, VTY_NEWLINE);
        }
    }
    else
        return CMD_SUCCESS;

    data = smap_get(&vswrow->mgmt_intf,SYSTEM_MGMT_INTF_MAP_DEFAULT_GATEWAY);
    if (data && (strcmp(data,MGMT_INTF_DEFAULT_IP) != 0))
    {
        vty_out(vty, "%4sdefault-gateway %s%s","",data, VTY_NEWLINE);
    }

    /* Ipv6 show running commands */

    data = smap_get(&vswrow->mgmt_intf,
            SYSTEM_MGMT_INTF_MAP_DEFAULT_GATEWAY_V6);
    if (data && (strcmp(data,MGMT_INTF_DEFAULT_IPV6) != 0))
    {
        vty_out(vty, "%4sdefault-gateway %s%s","",data, VTY_NEWLINE);
    }

    dns_1 = smap_get(&vswrow->mgmt_intf,SYSTEM_MGMT_INTF_MAP_DNS_SERVER_1);
    dns_2 = smap_get(&vswrow->mgmt_intf,SYSTEM_MGMT_INTF_MAP_DNS_SERVER_2);
    if (dns_1 && dns_2 && (strcmp(dns_1,MGMT_INTF_DEFAULT_IP) != 0) &&
            (strcmp(dns_2,MGMT_INTF_DEFAULT_IP) != 0))
    {
        vty_out(vty, "%4snameserver %s %s%s","", dns_1,dns_2, VTY_NEWLINE);
    }else if (dns_1 && (strcmp(dns_1,MGMT_INTF_DEFAULT_IP) != 0))
    {
        vty_out(vty, "%4snameserver %s %s","", dns_1, VTY_NEWLINE);
    }

    return CMD_SUCCESS;

}

DEFUN (cli_intf_show_run_intf_mgmt,
        cli_intf_show_run_intf_mgmt_cmd,
        "show running-config interface mgmt",
        SHOW_STR
        "Current running configuration\n"
        INTERFACE_STR
        "management interface\n")
{
    return cli_show_run_interface_mgmt_exec (self, vty);
}

int cli_show_xvr_exec (struct cmd_element *self, struct vty *vty,
        int flags, int argc, const char *argv[], bool brief)
{
    const struct ovsrec_interface *ifrow = NULL;
    const char *cur_state =NULL;
    struct shash sorted_interfaces;
    const struct shash_node **nodes;
    int idx, count;
    bool validIntf = false;

    struct string_pairs
    {
        char *key;
        char *string;
    };

    static struct string_pairs pluggable_keys [] = {
        { "connector_status", "Connector status" },
        { "vendor_name", "Vendor name" },
        { "vendor_part_number", "Part number" },
        { "vendor_revision", "Part revision" },
        { "vendor_serial_number", "Serial number" },
        { "supported_speeds", "Supported speeds" },
        { NULL, NULL }
    };

    shash_init(&sorted_interfaces);

    OVSREC_INTERFACE_FOR_EACH(ifrow, idl)
    {
        shash_add(&sorted_interfaces, ifrow->name, (void *)ifrow);
    }

    nodes = sort_interface(&sorted_interfaces);

    count = shash_count(&sorted_interfaces);

    for (idx = 0; idx < count; idx++)
    {
        ifrow = (const struct ovsrec_interface *)nodes[idx]->data;

        if ((NULL != argv[0]) && (0 != strcmp(argv[0],ifrow->name)))
        {
            continue;
        }

        if (strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_INTERNAL) == 0)
        {
            /* Skipping internal interfaces */
            continue;
        }
        validIntf = true;

        if (brief)
        {
            vty_out(vty, "%s", VTY_NEWLINE);
            vty_out(vty, "-----------------------------------------------%s",
                    VTY_NEWLINE);
            vty_out(vty, "Ethernet      Connector    Module     Module   %s",
                    VTY_NEWLINE);
            vty_out(vty, "Interface                  Type       Status   %s",
                    VTY_NEWLINE);
            vty_out(vty, "-----------------------------------------------%s",
                    VTY_NEWLINE);

            /* Display the brief information */
            vty_out (vty, " %-12s ", ifrow->name);
            bool present = false;
            cur_state = smap_get(&ifrow->hw_intf_info,
                    INTERFACE_HW_INTF_INFO_MAP_CONNECTOR);
            if (cur_state != NULL)
            {
                if (strcmp(cur_state,
                        INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_RJ45) ==0)
                {
                    vty_out(vty, "  RJ45       ");
                }
                else if (strcmp(cur_state,
                        INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_SFP_PLUS) ==0)
                {
                    vty_out(vty, "  SFP+       ");
                }
                else if (strcmp(cur_state,
                        INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_QSFP_PLUS) ==0)
                {
                    vty_out(vty, "  QSFP+      ");
                }
                else if (strcmp(cur_state,
                        INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_QSFP28) ==0)
                {
                    vty_out(vty, "  QSFP28     ");
                }
                else
                {
                    vty_out(vty, "             ");
                }
            }

            cur_state = smap_get(&ifrow->hw_intf_info,
                    INTERFACE_HW_INTF_INFO_MAP_PLUGGABLE);

            if (cur_state != NULL && strcmp(cur_state,
                    INTERFACE_HW_INTF_INFO_MAP_PLUGGABLE_TRUE) == 0)
            {
                cur_state = smap_get(&ifrow->pm_info,
                        INTERFACE_PM_INFO_MAP_CONNECTOR);
                if (cur_state != NULL)
                {
                    if (strcmp(cur_state,
                            OVSREC_INTERFACE_PM_INFO_CONNECTOR_ABSENT) == 0)
                    {
                        vty_out(vty, "%-11s", "--");
                        present = false;
                    }
                    else
                    {
                        vty_out(vty, "%-11s", cur_state);
                        present = true;
                    }
                }
                else
                {
                    vty_out(vty, "%-11s", "--");
                }

                cur_state = smap_get(&ifrow->pm_info,
                        INTERFACE_PM_INFO_MAP_CONNECTOR_STATUS);
                if (present && cur_state != NULL)
                {
                    vty_out(vty, "%-11s", cur_state);
                }
                else
                {
                    vty_out(vty, "%-11s", "--");
                }
            }
            else
            {
                vty_out(vty, "%-11s", "--");
                vty_out(vty, "%-11s", "--");
            }
            vty_out(vty, "%s", VTY_NEWLINE);
        }
        else
        {
            int i;

            /* Display transceiver information */
            vty_out (vty, "Interface %s:%s", ifrow->name, VTY_NEWLINE);

            if (strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_VLANSUBINT) == 0)
            {
                vty_out(vty, "Not supported by subinterface%s", VTY_NEWLINE);
            }
            cur_state = smap_get(&ifrow->hw_intf_info,
                    INTERFACE_HW_INTF_INFO_MAP_CONNECTOR);
            if (NULL != cur_state)
            {
                if (strcmp(cur_state,
                        INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_RJ45) == 0)
                {
                    vty_out(vty, " Connector: RJ45%s", VTY_NEWLINE);
                }
                else if (strcmp(cur_state,
                        INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_SFP_PLUS) == 0)
                {
                    vty_out(vty, " Connector: SFP+%s", VTY_NEWLINE);
                }
                else if (strcmp(cur_state,
                        INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_QSFP_PLUS) == 0)
                {
                    cur_state = smap_get(&ifrow->hw_intf_info,
                            INTERFACE_HW_INTF_INFO_MAP_SPLIT_4);
                    if (cur_state != NULL &&
                            strcmp(cur_state,
                               INTERFACE_HW_INTF_INFO_MAP_SPLIT_4_TRUE) == 0)
                    {
                        vty_out(vty, " Connector: QSFP+ (splittable)%s",
                                VTY_NEWLINE);
                    }
                    else
                    {
                        vty_out(vty, " Connector: QSFP+ %s", VTY_NEWLINE);
                    }
                }
                else if (strcmp(cur_state,
                        INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_QSFP28) == 0)
                {
                    cur_state = smap_get(&ifrow->hw_intf_info,
                            INTERFACE_HW_INTF_INFO_MAP_SPLIT_4);
                    if (cur_state != NULL &&
                            strcmp(cur_state,
                               INTERFACE_HW_INTF_INFO_MAP_SPLIT_4_TRUE) == 0)
                    {
                        vty_out(vty, " Connector: QSFP28 (splittable)%s",
                                VTY_NEWLINE);
                    }
                    else
                    {
                        vty_out(vty, " Connector: QSFP28 %s", VTY_NEWLINE);
                    }
                }
                else
                {
                    vty_out(vty, " Connector: %s%s", cur_state, VTY_NEWLINE);
                }
            }

            cur_state = smap_get(&ifrow->hw_intf_info,
                    INTERFACE_HW_INTF_INFO_MAP_PLUGGABLE);

            if (cur_state != NULL && strcmp(cur_state,
                    INTERFACE_HW_INTF_INFO_MAP_PLUGGABLE_TRUE) == 0)
            {
                cur_state = smap_get(&ifrow->pm_info,
                        INTERFACE_PM_INFO_MAP_CONNECTOR);
                if (cur_state != NULL)
                {
                    if (strcmp(cur_state,
                            OVSREC_INTERFACE_PM_INFO_CONNECTOR_ABSENT) == 0)
                    {
                        vty_out(vty, " Transceiver module: not present%s",
                                VTY_NEWLINE);
                    }
                    else
                    {
                        vty_out(vty, " Transceiver module: %s%s", cur_state,
                                VTY_NEWLINE);
                        for (i = 0; pluggable_keys[i].key != NULL; i++)
                        {
                            vty_out(vty, " %s: ", pluggable_keys[i].string);
                            cur_state = smap_get(&ifrow->pm_info,
                                    pluggable_keys[i].key);
                            if (cur_state != NULL)
                            {
                                vty_out(vty, "%s%s", cur_state, VTY_NEWLINE);
                            }
                            else
                            {
                                vty_out(vty, "%s", VTY_NEWLINE);
                            }
                        }
                    }
                }
                else
                {
                    vty_out(vty,
                            " Transceiver module: no information available%s",
                            VTY_NEWLINE);
                }
            }
            vty_out (vty, "%s", VTY_NEWLINE);
        }
    }

    shash_destroy(&sorted_interfaces);
    free(nodes);

    if (validIntf)
    {
        return CMD_SUCCESS;
    }
    else
    {
        vty_out (vty, "Invalid switch interface ID.%s", VTY_NEWLINE);
        return CMD_OVSDB_FAILURE;
    }

}


void
show_lacp_interfaces_brief (struct vty *vty, const char *argv[])
{
    const struct ovsrec_port *lag_port = NULL;
    const struct ovsrec_interface *if_row = NULL;
    const struct ovsdb_datum *datum;

    int64_t lag_speed = 0;

    // Index for loops
    int interface_index = 0;

    OVSREC_PORT_FOR_EACH(lag_port, idl)
    {
        if ((NULL != argv[0]) && (0 != strcmp(argv[0], lag_port->name)))
        {
            continue;
        }

        if(strncmp(lag_port->name, LAG_PORT_NAME_PREFIX, LAG_PORT_NAME_PREFIX_LENGTH) != 0)
        {
            continue;
        }

        vty_out(vty, " %-15s ", lag_port->name);
        /* Display vid for an lag interface */
        if (lag_port->vlan_tag != NULL ) {
            vty_out(vty, "%-8ld", (int64_t)ops_port_get_tag(lag_port)); /*vid */
        }
        else {
            vty_out(vty, "--      "); /*vid */
        }
        vty_out(vty, "--  "); /*type */

        /* Display vlan mode for an lag interface */
        if (lag_port->vlan_mode == NULL) {
            vty_out(vty, "--      ");
        }
        else {
            /* Access mode */
            if (strncmp(lag_port->vlan_mode, OVSREC_PORT_VLAN_MODE_ACCESS,
                     strlen(OVSREC_PORT_VLAN_MODE_ACCESS)) == 0) {
                vty_out(vty, "%-8s", OVSREC_PORT_VLAN_MODE_ACCESS); /*Access mode */
             }
             else {
             /* Trunk mode - trunk, native-tagged or native-untagged*/
                vty_out(vty, "%-8s", OVSREC_PORT_VLAN_MODE_TRUNK); /* Trunk mode */
             }
        }

        vty_out(vty, "--     ");/* Status */
        vty_out(vty, "--                       "); /*Reason*/

        /* Speed calculation: Adding speed of all aggregated interfaces*/
        lag_speed = 0;
        for (interface_index = 0; interface_index < lag_port->n_interfaces; interface_index++)
        {
            if_row = lag_port->interfaces[interface_index];

            datum = ovsrec_interface_get_link_speed(if_row, OVSDB_TYPE_INTEGER);
            if ((NULL != datum) && (datum->n >0))
            {
                lag_speed += datum->keys[0].integer;
            }
        }
        if(lag_speed == 0)
        {
            vty_out(vty, " %-5s", "auto");
        }
        else
        {
            vty_out(vty, " %-6ld", lag_speed/1000000);
        }

        vty_out(vty, "   -- ");  /* Port channel */
        vty_out(vty, "%s", VTY_NEWLINE);
    }
}

void
show_lacp_queue_stats (struct vty *vty, char* interface_queue_stats_keys[],
                                                        const char *argv[])
{
    const struct ovsrec_port *lag_port = NULL;
    const struct ovsrec_interface *if_row = NULL;
    const struct ovsdb_datum *datum;

    int64_t lag_speed = 0;

    // Indexes for loops
    int interface_index, q = 0;

    const struct ovsdb_datum *datum_array[QOS_QUEUE_STATS];
    // Array to keep the statistics for each lag while adding the
    // stats for each interface in the lag.
    int64_t lag_queue_stats[QOS_QUEUE_STATS][QOS_MAX_QUEUES] = {{0}};

    OVSREC_PORT_FOR_EACH(lag_port, idl)
    {
        if ((NULL != argv[0]) && (0 != strcmp(argv[0],lag_port->name)))
        {
            continue;
        }

        if(strncmp(lag_port->name, LAG_PORT_NAME_PREFIX, LAG_PORT_NAME_PREFIX_LENGTH) != 0)
        {
            continue;
        }

        // Reinitialize queue stats aggregator array
        memset (lag_queue_stats, 0, sizeof lag_queue_stats);

        vty_out(vty, "Aggregate-name %s %s", lag_port->name, VTY_NEWLINE);
        vty_out(vty, " Aggregated-interfaces : ");

        lag_speed = 0;
        for (interface_index = 0; interface_index < lag_port->n_interfaces; interface_index++)
        {
            if_row = lag_port->interfaces[interface_index];
            vty_out(vty, "%s ", if_row->name);

            datum = ovsrec_interface_get_link_speed(if_row, OVSDB_TYPE_INTEGER);
            if ((NULL!=datum) && (datum->n >0))
            {
                lag_speed += datum->keys[0].integer;
            }

            datum_array[0] = ovsrec_interface_get_queue_tx_bytes(if_row,
                                 OVSDB_TYPE_INTEGER, OVSDB_TYPE_INTEGER);
            if ((NULL == datum_array[0]) || (datum_array[0]->n == 0))
            {
                vty_out(vty, " No queue tx bytes statistics available%s", VTY_NEWLINE);
                vty_out(vty, "%s", VTY_NEWLINE);
                continue;
            }

            datum_array[1] = ovsrec_interface_get_queue_tx_packets(if_row,
                                   OVSDB_TYPE_INTEGER, OVSDB_TYPE_INTEGER);
            if ((NULL == datum_array[1]) || (datum_array[1]->n == 0))
            {
                vty_out(vty, " No queue tx packets statistics available%s", VTY_NEWLINE);
                vty_out(vty, "%s", VTY_NEWLINE);
                continue;
            }

            datum_array[2] = ovsrec_interface_get_queue_tx_errors(if_row,
                                  OVSDB_TYPE_INTEGER, OVSDB_TYPE_INTEGER);

            if ((NULL==datum_array[2]) || (datum_array[2]->n == 0))
            {
                vty_out(vty, " No queue tx errors statistics available%s", VTY_NEWLINE);
                vty_out(vty, "%s", VTY_NEWLINE);
                continue;
            }

            for (q=0; q<QOS_MAX_QUEUES; q++)
            {
                lag_queue_stats[0][q] += datum_array[0]->values[q].integer;
                lag_queue_stats[1][q] += datum_array[1]->values[q].integer;
                lag_queue_stats[2][q] += datum_array[2]->values[q].integer;
            }

        }

        vty_out(vty, "%s", VTY_NEWLINE);
        vty_out(vty, " Speed %ld Mb/s %s",lag_speed/1000000 , VTY_NEWLINE);
        vty_out(vty, "     %20s  %20s  %20s", interface_queue_stats_keys[0],
                                              interface_queue_stats_keys[1],
                                              interface_queue_stats_keys[2]);
        vty_out(vty, "%s", VTY_NEWLINE);

        for (q=0; q<QOS_MAX_QUEUES; q++)
        {
            vty_out(vty, " Q%d", q);
            vty_out(vty, "  %20ld", lag_queue_stats[0][q]);
            vty_out(vty, "  %20ld", lag_queue_stats[1][q]);
            vty_out(vty, "  %20ld", lag_queue_stats[2][q]);
            vty_out(vty, "%s", VTY_NEWLINE);
        }
        vty_out(vty, "%s", VTY_NEWLINE);
    }
}

void
show_lacp_interfaces (struct vty *vty, char* interface_statistics_keys[],
                      const char *argv[])
{
    const struct ovsrec_port *lag_port = NULL;
    const struct ovsrec_interface *if_row = NULL;
    const char *aggregate_mode = NULL;
    const struct ovsdb_datum *datum;
    unsigned int index;
    const char* ipv4_address = NULL;
    const char* ipv6_address = NULL;
    int i = 0;

    int64_t lag_speed = 0;

    // Indexes for loops
    int interface_index = 0;
    int stat_index = 0;
    bool lag_found = false;

    // Array to keep the statistics for each lag while adding the
    // stats for each interface in the lag.
    uint64_t lag_statistics [12] = {0};

    // Aggregation-key variables
    size_t aggr_key_len = 6;
    char aggr_key[aggr_key_len];

    OVSREC_PORT_FOR_EACH(lag_port, idl)
    {
        union ovsdb_atom atom;

        if ((NULL != argv[0]) && (0 != strcmp(argv[0],lag_port->name)))
        {
            continue;
        }

        if(strncmp(lag_port->name, LAG_PORT_NAME_PREFIX, LAG_PORT_NAME_PREFIX_LENGTH) != 0)
        {
            continue;
        }
        lag_found = true;

        vty_out(vty, "Aggregate-name %s %s", lag_port->name, VTY_NEWLINE);
        vty_out(vty, " Aggregated-interfaces : ");

        lag_speed = 0;
        for (interface_index = 0; interface_index < lag_port->n_interfaces; interface_index++)
        {
            if_row = lag_port->interfaces[interface_index];
            vty_out(vty, "%s ", if_row->name);

            datum = ovsrec_interface_get_link_speed(if_row, OVSDB_TYPE_INTEGER);
            if ((NULL!=datum) && (datum->n >0))
            {
                lag_speed += datum->keys[0].integer;
            }

            datum = ovsrec_interface_get_statistics(if_row,
                                OVSDB_TYPE_STRING, OVSDB_TYPE_INTEGER);

            // Adding statistic value for each interface in the lag
            for (stat_index = 0; stat_index < sizeof(lag_statistics)/sizeof(int64_t); stat_index++)
            {
                atom.string = interface_statistics_keys[stat_index];
                index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
                lag_statistics[stat_index] += (index == UINT_MAX)? 0 : datum->values[index].integer;
            }
        }
        vty_out(vty, "%s", VTY_NEWLINE);

        /* Retrieve aggregation-key from lag name */
        snprintf(aggr_key,
                 aggr_key_len,
                 "%s",
                 lag_port->name + LAG_PORT_NAME_PREFIX_LENGTH);

        vty_out(vty, " Aggregation-key : %s", aggr_key);
        vty_out(vty, "%s", VTY_NEWLINE);

        aggregate_mode = lag_port->lacp;
        if(aggregate_mode)
            vty_out(vty, " Aggregate mode : %s %s", aggregate_mode, VTY_NEWLINE);
        ipv4_address = lag_port->ip4_address;
        if (ipv4_address){
            vty_out(vty, " IPv4 address %s %s", ipv4_address, VTY_NEWLINE);
        }
        for (i = 0; i < lag_port->n_ip4_address_secondary; i++) {
            vty_out(vty, " IPv4 address %s secondary%s",
                    lag_port->ip4_address_secondary[i],
                    VTY_NEWLINE);
        }
        ipv6_address = lag_port->ip6_address;
        if (ipv6_address){
            vty_out(vty, " IPv6 address %s %s", ipv6_address, VTY_NEWLINE);
        }
        for (i = 0; i < lag_port->n_ip6_address_secondary; i++) {
            vty_out(vty, " IPv6 address %s secondary%s",
                    lag_port->ip6_address_secondary[i],
                    VTY_NEWLINE);
        }

        vty_out(vty, " Speed %ld Mb/s %s",lag_speed/1000000 , VTY_NEWLINE);

        qos_trust_port_show(lag_port, lag_port->name);
        qos_apply_port_show(lag_port, lag_port->name);
        qos_cos_port_show(lag_port, lag_port->name);
        qos_dscp_port_show(lag_port, lag_port->name);

        vty_out(vty, " RX%s", VTY_NEWLINE);
        vty_out(vty, "   %10lu input packets  ", lag_statistics[0]);
        vty_out(vty, "   %10lu bytes  ",lag_statistics[1]);
        vty_out(vty, "%s", VTY_NEWLINE);

        vty_out(vty, "   %10lu input error    ",lag_statistics[8]);
        vty_out(vty, "   %10lu dropped  ",lag_statistics[4]);
        vty_out(vty, "%s", VTY_NEWLINE);

        vty_out(vty, "   %10lu CRC/FCS  ",lag_statistics[7]);
        vty_out(vty, "%s", VTY_NEWLINE);
        vty_out(vty, " TX%s", VTY_NEWLINE);

        vty_out(vty, "   %10lu output packets ",lag_statistics[2]);
        vty_out(vty, "   %10lu bytes  ",lag_statistics[3]);
        vty_out(vty, "%s", VTY_NEWLINE);

        vty_out(vty, "   %10lu input error    ",lag_statistics[11]);
        vty_out(vty, "   %10lu dropped  ",lag_statistics[9]);
        vty_out(vty, "%s", VTY_NEWLINE);

        vty_out(vty, "   %10lu collision  ",lag_statistics[10]);
        vty_out(vty, "%s", VTY_NEWLINE);
    }

    if (lag_found) {
        vty_out(vty, "%s", VTY_NEWLINE);
    }
    else {
        vty_out(vty, "%% Command incomplete.%s", VTY_NEWLINE);
      }
}

void
show_interface_stats(struct vty *vty, const struct ovsrec_interface *ifrow,
        char *interface_statistics_keys[])
{
    const struct ovsdb_datum *datum;
    union ovsdb_atom atom;
    unsigned int index;
    bool l3_intf = (check_iface_in_vrf(ifrow->name) &&
            port_find(ifrow->name));

    datum = ovsrec_interface_get_statistics(ifrow, OVSDB_TYPE_STRING, OVSDB_TYPE_INTEGER);
    if (NULL==datum)
        return;

    vty_out(vty, " RX%s", VTY_NEWLINE);

    atom.string = interface_statistics_keys[0];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "   %10ld input packets  ",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    atom.string = interface_statistics_keys[1];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "   %10ld bytes  ",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    vty_out(vty, "%s", VTY_NEWLINE);

    atom.string = interface_statistics_keys[8];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "   %10ld input error    ",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    atom.string = interface_statistics_keys[4];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "   %10ld dropped  ",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    vty_out(vty, "%s", VTY_NEWLINE);

    atom.string = interface_statistics_keys[7];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "   %10ld CRC/FCS  ",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    vty_out(vty, "%s", VTY_NEWLINE);

    if (l3_intf)
        show_l3_interface_rx_stats(vty, datum);

    vty_out(vty, " TX%s", VTY_NEWLINE);

    atom.string = interface_statistics_keys[2];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "   %10ld output packets ",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    atom.string = interface_statistics_keys[3];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "   %10ld bytes  ",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    vty_out(vty, "%s", VTY_NEWLINE);

    atom.string = interface_statistics_keys[11];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "   %10ld input error    ",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    atom.string = interface_statistics_keys[9];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "   %10ld dropped  ",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    vty_out(vty, "%s", VTY_NEWLINE);

    atom.string = interface_statistics_keys[10];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "   %10ld collision  ",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    vty_out(vty, "%s", VTY_NEWLINE);

    if (l3_intf)
        show_l3_interface_tx_stats(vty, datum);

    vty_out(vty, "%s", VTY_NEWLINE);
}

static void
show_interface_status(struct vty *vty, const const struct ovsrec_interface *ifrow,
        bool internal_if, bool brief)
{
    const char *state_value;
    if(brief)
    {
        const struct ovsrec_port *port_row;
        port_row = port_check_and_add(ifrow->name, false, false, NULL);
        vty_out (vty, " %-15s ", ifrow->name);

        /* Display vlan mode and vid for an L3 interface,
         * port table by default is not populated unless
         * entered into interface mode
         */
        if (port_row == NULL ||
            (port_row->vlan_tag == NULL && port_row->vlan_mode == NULL)) {
            vty_out(vty, "--      "); /*vid */
            vty_out(vty, "eth  "); /*type */
            vty_out(vty, "%-7s", VLAN_MODE_ROUTED);
        }
        /* Display vlan mode and vid for interface VLAN*/
        else if (port_row->vlan_tag != NULL && port_row->vlan_mode == NULL) {
            vty_out(vty, "--      "); /*vid */
            vty_out(vty, "eth  "); /*type */
            vty_out(vty, "       "); /* mode - routed or not */
        }
        /* Display vlan mode and vid for an l2 interface */
        else if (port_row->vlan_tag != NULL && port_row->vlan_mode != NULL) {
            vty_out(vty, "%-8ld", (int64_t)ops_port_get_tag(port_row)); /*vid */
            vty_out(vty, "eth  "); /*type */
            if (strncmp(port_row->vlan_mode, OVSREC_PORT_VLAN_MODE_ACCESS,
                        strlen(OVSREC_PORT_VLAN_MODE_ACCESS)) == 0){
                /* Default in access mode */
                vty_out(vty, "%-7s", OVSREC_PORT_VLAN_MODE_ACCESS);
            }
            else {
                /* Trunk mode - trunk, native-tagged or native-untagged*/
                 vty_out(vty, "%-7s", OVSREC_PORT_VLAN_MODE_TRUNK);
            }
        }

        state_value = smap_get(&ifrow->user_config,
                               INTERFACE_USER_CONFIG_MAP_ADMIN);
        if ((NULL == state_value) ||
                (strcmp(state_value,
                        OVSREC_INTERFACE_USER_CONFIG_ADMIN_DOWN) == 0))
        {
            if(internal_if)
                vty_out (vty, "%-6s ", "down");
            else
                vty_out (vty, "%-6s ", ifrow->link_state);
            vty_out (vty, "Administratively down    ");
        }
        else
        {
            if(internal_if)
                vty_out (vty, "%-6s ", "up");
            else
                vty_out (vty, "%-6s ", ifrow->link_state);
            vty_out (vty, "                         ");
        }
    }
    else
    {
        vty_out (vty, "Interface %s is ", ifrow->name);
        state_value = smap_get(&ifrow->user_config,
                               INTERFACE_USER_CONFIG_MAP_ADMIN);
        if ((NULL == state_value)
                || strcmp(state_value,
                        OVSREC_INTERFACE_USER_CONFIG_ADMIN_DOWN) == 0)
        {
            if(internal_if)
                vty_out (vty, "down ");
            else
                vty_out (vty, "%s ", ifrow->link_state);
            vty_out (vty, "(Administratively down) %s", VTY_NEWLINE);
            vty_out (vty, " Admin state is down%s",
                    VTY_NEWLINE);
        }
        else
        {
            if(internal_if)
                vty_out (vty, "up %s", VTY_NEWLINE);
            else
                vty_out (vty, "%s %s", ifrow->link_state, VTY_NEWLINE);
            vty_out (vty, " Admin state is up%s", VTY_NEWLINE);
        }

        if (ifrow->error != NULL)
        {
            vty_out (vty, " State information: %s%s",
                    ifrow->error, VTY_NEWLINE);
        }
    }
}

int
cli_show_interface_queue_stats (struct cmd_element *self, struct vty *vty,
                                         int argc, const char *argv[])
{
    const struct ovsrec_interface *ifrow = NULL;
    struct shash sorted_interfaces;
    const struct shash_node **nodes;
    int idx, count, q = 0;

    static char *interface_queue_stats_keys [] = {
        "Tx Bytes",
        "Tx Packets",
        "Tx Errors"
    };

    const struct ovsdb_datum *datum[QOS_QUEUE_STATS];

    shash_init(&sorted_interfaces);

    OVSREC_INTERFACE_FOR_EACH(ifrow, idl)
    {
        if ((NULL != argv[0]) && (0 != strcmp(argv[0], ifrow->name)))
        {
            continue;
        }

        if (strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_SYSTEM) != 0)
        {
            continue;
        }

        shash_add(&sorted_interfaces, ifrow->name, (void *)ifrow);
    }

    nodes = sort_interface(&sorted_interfaces);

    count = shash_count(&sorted_interfaces);

    for (idx = 0; idx < count; idx++)
    {
        ifrow = (const struct ovsrec_interface *)nodes[idx]->data;


        vty_out (vty, "Interface %s is %s ", ifrow->name, ifrow->link_state);
        const char *state_value = smap_get(&ifrow->user_config,
                               INTERFACE_USER_CONFIG_MAP_ADMIN);
        if ((NULL != state_value) &&
                (strcmp(state_value,
                        OVSREC_INTERFACE_USER_CONFIG_ADMIN_DOWN) == 0))
        {
            vty_out (vty, "(Administratively down) %s", VTY_NEWLINE);
            vty_out (vty, " Admin state is down%s",
                        VTY_NEWLINE);
        }
        else
        {
            vty_out (vty, "%s", VTY_NEWLINE);
            vty_out (vty, " Admin state is up%s", VTY_NEWLINE);
        }

        datum[0] = ovsrec_interface_get_queue_tx_bytes(ifrow,
                       OVSDB_TYPE_INTEGER, OVSDB_TYPE_INTEGER);
        if ((NULL == datum[0]) || (datum[0]->n == 0))
        {
            vty_out(vty, " No queue tx bytes statistics available%s", VTY_NEWLINE);
            vty_out(vty, "%s", VTY_NEWLINE);
            continue;
        }

        datum[1] = ovsrec_interface_get_queue_tx_packets(ifrow,
                       OVSDB_TYPE_INTEGER, OVSDB_TYPE_INTEGER);
        if ((NULL==datum[1]) || (datum[1]->n == 0))
        {
            vty_out(vty, " No queue tx packets statistics available%s", VTY_NEWLINE);
            vty_out(vty, "%s", VTY_NEWLINE);
            continue;
        }

        datum[2] = ovsrec_interface_get_queue_tx_errors(ifrow,
                       OVSDB_TYPE_INTEGER, OVSDB_TYPE_INTEGER);

        if ((NULL==datum[2]) || (datum[2]->n == 0))
        {
            vty_out(vty, " No queue tx errors statistics available%s", VTY_NEWLINE);
            vty_out(vty, "%s", VTY_NEWLINE);
            continue;
        }

        vty_out(vty, "     %20s  %20s  %20s", interface_queue_stats_keys[0],
                                             interface_queue_stats_keys[1],
                                             interface_queue_stats_keys[2]);
        vty_out(vty, "%s", VTY_NEWLINE);

        for (q=0; q<QOS_MAX_QUEUES; q++)
        {
            vty_out(vty, " Q%d", q);
            vty_out(vty, "  %20ld", datum[0]->values[q].integer);
            vty_out(vty, "  %20ld", datum[1]->values[q].integer);
            vty_out(vty, "  %20ld", datum[2]->values[q].integer);
            vty_out(vty, "%s", VTY_NEWLINE);
        }
        vty_out(vty, "%s", VTY_NEWLINE);
    }

    shash_destroy(&sorted_interfaces);
    free(nodes);

    show_lacp_queue_stats (vty, interface_queue_stats_keys, argv);

    return CMD_SUCCESS;
}

void
display_header(bool brief) {
    if (brief) {
        /* Display the brief information */
        vty_out(vty, "%s", VTY_NEWLINE);
        vty_out(vty, "--------------------------------------------------"
                     "---------------------------------%s", VTY_NEWLINE);
        vty_out(vty, "Ethernet         VLAN    Type Mode   Status  Reason  "
                     "                 Speed    Port%s", VTY_NEWLINE);
        vty_out(vty, "Interface                                            "
                     "                 (Mb/s)   Ch#%s", VTY_NEWLINE);
        vty_out(vty, "--------------------------------------------------"
                     "---------------------------------%s", VTY_NEWLINE);
    }
    else {
        vty_out (vty, "%s", VTY_NEWLINE);
    }
}

int
cli_show_interface_exec (struct cmd_element *self, struct vty *vty,
        int flags, int argc, const char *argv[], bool brief)
{
    const struct ovsrec_interface *ifrow = NULL;
    const char *cur_state = NULL;
    struct shash sorted_interfaces;
    const struct shash_node **nodes;
    int idx, count;
    const struct ovsrec_port *port_row;
    bool internal_if = false;
    bool isLag = true;
    const struct ovsdb_datum *datum;
    const char *cur_duplex = NULL;
    const char *cur_mtu = NULL;
    const char *cur_flow_control = NULL;
    static char *interface_statistics_keys [] = {
        "rx_packets",
        "rx_bytes",
        "tx_packets",
        "tx_bytes",
        "rx_dropped",
        "rx_frame_err",
        "rx_over_err",
        "rx_crc_err",
        "rx_errors",
        "tx_dropped",
        "collisions",
        "tx_errors"
    };
    int64_t intVal = 0;
    const char *user_config_speed = NULL;

    shash_init(&sorted_interfaces);

    OVSREC_INTERFACE_FOR_EACH(ifrow, idl)
    {
        const char *state_value;

        if(!ifrow->split_parent)
        {
            /*Parent (orphan) interface */
            state_value = smap_get(&ifrow->user_config,
                                   INTERFACE_USER_CONFIG_MAP_LANE_SPLIT);
            if(state_value
                   &&  !strcmp(state_value,
                          INTERFACE_USER_CONFIG_MAP_LANE_SPLIT_SPLIT))
            {
                VLOG_DBG("Skipped parent int %s, split_config= %s", ifrow->name, state_value);

                /* can't show parent config, when the interface is split */
                if ((argv[0] != NULL) && !strcmp(ifrow->name, argv[0])) {
                    vty_out (vty, "Interface %s is split. Check the child interfaces"
                                  " for configuration. %s", ifrow->name,
                                  VTY_NEWLINE);
                    return CMD_SUCCESS;
                }

                continue;
           }
        }
        else
        {   /*Child interface */
            /*Check if the parent was split */
            state_value = smap_get(&ifrow->split_parent->user_config,
                                      INTERFACE_USER_CONFIG_MAP_LANE_SPLIT);
            if(!state_value
                  || strcmp(state_value,
                                     INTERFACE_USER_CONFIG_MAP_LANE_SPLIT_SPLIT))
            {
                VLOG_DBG("Skipped child int %s, split_config of parent= %s", ifrow->name, state_value);

               /* can't show child interface config when parent is not split */
               if ((argv[0] != NULL) && !strcmp(ifrow->name, argv[0])) {
                    vty_out (vty, "Parent interface of %s is not split. "
                                  "Check the parent interface for "
                                  "configuration.%s", ifrow->name, VTY_NEWLINE);
                    return CMD_SUCCESS;
                }

                continue;
            }
        }

        if ((NULL != argv[0]) && (0 != strcmp(argv[0],ifrow->name)))
        {
            continue;
        }
        else if ((NULL != argv[0]) &&
            (strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_VLANSUBINT) == 0))
        {
             display_header(brief);

             cli_show_subinterface_row(ifrow, brief);
             shash_destroy(&sorted_interfaces);
             return CMD_SUCCESS;
        }

        if (strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_SYSTEM) != 0 &&
                strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_INTERNAL) != 0)
        {
            continue;
        }

        shash_add(&sorted_interfaces, ifrow->name, (void *)ifrow);
    }

    display_header(brief);

    nodes = sort_interface(&sorted_interfaces);
    count = shash_count(&sorted_interfaces);

    for (idx = 0; idx < count; idx++)
    {
        ifrow = (const struct ovsrec_interface *)nodes[idx]->data;
        internal_if = (strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_INTERNAL) == 0) ? true : false;

        if (brief)
        {

            show_interface_status(vty, ifrow, internal_if, brief);

            if ((NULL == ifrow->link_state) ||
                (strcmp(OVSREC_INTERFACE_USER_CONFIG_ADMIN_DOWN, ifrow->link_state) == 0) )
            {
                user_config_speed = smap_get(&ifrow->user_config, INTERFACE_USER_CONFIG_MAP_SPEEDS);
                if (user_config_speed != NULL) {
                    vty_out(vty,"%-6s", user_config_speed);
                }
                else
                {
                    vty_out(vty," --    ");
                }
            }
            else
            {
                intVal = 0;
                datum = ovsrec_interface_get_link_speed(ifrow, OVSDB_TYPE_INTEGER);
                if ((NULL != datum) && (datum->n >0))
                {
                    intVal = datum->keys[0].integer;
                }

                if(intVal == 0)
                {
                    vty_out(vty, " %-6s", "auto");
                }
                else
                {
                    vty_out(vty, " %-6ld", intVal/1000000);
                }
            }
            vty_out(vty, "   -- ");  /* Port channel */
            vty_out (vty, "%s", VTY_NEWLINE);
        }
        else
        {
            intVal = 0;
            isLag = false;
            show_interface_status(vty, ifrow, internal_if, brief);

            vty_out (vty, " Hardware: Ethernet, MAC Address: %s %s",
                    ifrow->mac_in_use, VTY_NEWLINE);

            port_row = port_find(ifrow->name);
            if (port_row && smap_get(&port_row->other_config,
                  PORT_OTHER_CONFIG_MAP_PROXY_ARP_ENABLED))
            {
                vty_out(vty, " Proxy ARP is enabled%s", VTY_NEWLINE);
            }

            if (port_row && smap_get(&port_row->other_config,
                  PORT_OTHER_CONFIG_MAP_LOCAL_PROXY_ARP_ENABLED))
            {
                vty_out(vty, " Local Proxy ARP is enabled%s", VTY_NEWLINE);
            }

            /* Displaying ipv4 and ipv6 primary and secondary addresses*/
            show_ip_addresses(ifrow->name, vty);

            if(!internal_if)
            {
                cur_mtu = smap_get(&ifrow->user_config,
                                      INTERFACE_USER_CONFIG_MAP_MTU);
                if (NULL != cur_mtu) {
                    intVal = atoi(cur_mtu);
                }
                else {
                    intVal = atoi(INTERFACE_USER_CONFIG_MAP_MTU_DEFAULT);
                }

                vty_out(vty, " MTU %ld %s", intVal, VTY_NEWLINE);

                cur_duplex = smap_get(&ifrow->user_config,
                                      INTERFACE_USER_CONFIG_MAP_DUPLEX);
                if ((NULL == cur_duplex) ||
                        !strcmp(cur_duplex, "full"))
                {
                    vty_out(vty, " Full-duplex %s", VTY_NEWLINE);
                }
                else
                {
                    vty_out(vty, " Half-duplex %s", VTY_NEWLINE);
                }

                const struct ovsrec_port* port_row = port_find(ifrow->name);
                qos_trust_port_show(port_row, ifrow->name);
                qos_apply_port_show(port_row, ifrow->name);
                qos_cos_port_show(port_row, ifrow->name);
                qos_dscp_port_show(port_row, ifrow->name);

                intVal = 0;
                datum = ovsrec_interface_get_link_speed(ifrow, OVSDB_TYPE_INTEGER);
                if ((NULL != datum) && (datum->n >0))
                {
                    intVal = datum->keys[0].integer;
                }
                vty_out(vty, " Speed %ld Mb/s %s",intVal/1000000 , VTY_NEWLINE);

                cur_state = smap_get(&ifrow->user_config,
                                       INTERFACE_USER_CONFIG_MAP_AUTONEG);
                if ((NULL == cur_state) ||
                    strcmp(cur_state, "off") !=0)
                {
                    vty_out(vty, " Auto-Negotiation is turned on %s", VTY_NEWLINE);
                }
                else
                {
                    vty_out(vty, " Auto-Negotiation is turned off %s",
                        VTY_NEWLINE);
                }

                cur_flow_control = smap_get(&ifrow->user_config,
                                      INTERFACE_USER_CONFIG_MAP_PAUSE);
                if (NULL != cur_flow_control)
                {
                    if (strcmp(cur_flow_control,
                        INTERFACE_USER_CONFIG_MAP_PAUSE_NONE) == 0)

                    {
                        vty_out(vty, " Input flow-control is off, "
                            "output flow-control is off%s",VTY_NEWLINE);
                    }
                    else if (strcmp(cur_flow_control,
                        INTERFACE_USER_CONFIG_MAP_PAUSE_RX) == 0)
                    {
                        vty_out(vty, " Input flow-control is on, "
                            "output flow-control is off%s",VTY_NEWLINE);
                    }
                    else if (strcmp(cur_flow_control,
                        INTERFACE_USER_CONFIG_MAP_PAUSE_TX) == 0)
                    {
                        vty_out(vty, " Input flow-control is off, "
                            "output flow-control is on%s",VTY_NEWLINE);
                    }
                    else
                    {
                        vty_out(vty, " Input flow-control is on, "
                            "output flow-control is on%s",VTY_NEWLINE);
                    }
                }
                else
                {
                    vty_out(vty, " Input flow-control is off, "
                        "output flow-control is off%s",VTY_NEWLINE);
                }
            }

            /* show sFlow config, if present */
            show_sflow_config(ifrow->name, " ", true);

            if(internal_if)
                show_l3_stats(vty, ifrow);
            else
                show_interface_stats(vty, ifrow, interface_statistics_keys);
        }
    }

    shash_destroy(&sorted_interfaces);
    free(nodes);

    if(brief)
    {
        show_lacp_interfaces_brief(vty, argv);
    }
    else if (isLag)
    {
        show_lacp_interfaces(vty, interface_statistics_keys, argv);
    }

    return CMD_SUCCESS;
}

DEFUN (vtysh_interface,
      vtysh_interface_cmd,
      "interface IFNAME",
      "Select an interface to configure\n"
      "Interface's name\n")
{
    static char ifnumber[MAX_IFNAME_LENGTH];
    char ifnumber_temp[MAX_IFNAME_LENGTH];
    const struct ovsrec_interface *if_row = NULL;
    const struct ovsrec_vlan *vlan_row = NULL;
    uint16_t flag = 1;
    int vlan_id;
    unsigned int parent_if_number;
    unsigned int split_child_id;

    /* User not allowed to configure default interface bridge_normal */
    if (strcmp(argv[0], "bridge_normal") == 0) {
        vty_out (vty, "Configuration of %s (default) not allowed.%s",
                       argv[0],VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    } else if (strncmp(argv[0], "lag", 3) == 0) {
        vty_out (vty, "Unknown interface %s. %s",
                 argv[0], VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
    }

    /* check if a split child interface can be configured */
    if (strchr(argv[0], '-')) {
        sscanf (argv[0],"%u-%u",&parent_if_number, &split_child_id);

        char if_name_str[5];
        sprintf(if_name_str, "%u", parent_if_number);

        OVSREC_INTERFACE_FOR_EACH (if_row, idl) {
           if (!strcmp (if_row->name, if_name_str)) {
                if (!is_parent_interface_split (if_row)) {
                     vty_out (vty, "Configuration not allowed as parent "
                              "interface of %s is not split.%s",
                              argv[0],VTY_NEWLINE);
                     return CMD_ERR_NOTHING_TODO;
                }
           }
        }
    }

    if (strchr(argv[0], '.')) {
        return create_sub_interface(argv[0]);
    }
    else {
        vty->node = INTERFACE_NODE;
    }

    if (verify_ifname((char *)argv[0]))
    {
        vty->node = VLAN_INTERFACE_NODE;
        GET_VLANIF(ifnumber_temp, argv[0]);
        vlan_id = atoi(argv[0] + 4);
        OVSREC_VLAN_FOR_EACH(vlan_row, idl)
        {
            if (vlan_row->id == vlan_id)
            {
                if (create_vlan_interface(ifnumber_temp) == CMD_OVSDB_FAILURE)
                {
                    return CMD_OVSDB_FAILURE;
                }
                break;
            }
        }
        if (vlan_row == NULL) {
            vty_out(vty, "VLAN %d should be created before creating "
                    "interface VLAN%d.%s",vlan_id, vlan_id, VTY_NEWLINE);
            vty->node = CONFIG_NODE;
            return CMD_ERR_NOTHING_TODO;
        }
    }
    else if (strncmp(argv[0], LAG_PORT_NAME_PREFIX, LAG_PORT_NAME_PREFIX_LENGTH) == 0) {
            vty_out(vty, "%% Unknown command.%s", VTY_NEWLINE);
            return CMD_SUCCESS;
         } else if (strlen(argv[0]) < MAX_IFNAME_LENGTH) {
                   strncpy(ifnumber_temp, argv[0], MAX_IFNAME_LENGTH);

                   OVSREC_INTERFACE_FOR_EACH (if_row, idl)
                   {
                       if (strcmp (if_row->name, ifnumber_temp) == 0) {
                           if ((if_row->error != NULL) &&
                               ((strcmp(if_row->error, "lanes_split")) == 0)) {
                              vty_out(vty, "Interface Warning : Split Interface\n");
                              flag = 0;
                              break;
                           }
                       }
                   }
                   if (flag) {
                      default_port_add(ifnumber_temp);
                   }
                }
                else {
                    return CMD_ERR_NO_MATCH;
                }
    VLOG_DBG("%s ifnumber = %s\n", __func__, ifnumber_temp);
    strncpy(ifnumber, ifnumber_temp, MAX_IFNAME_LENGTH);
    vty->index = ifnumber;
    return CMD_SUCCESS;
}

DEFUN (no_vtysh_interface,
      no_vtysh_interface_cmd,
      "no interface IFNAME",
      NO_STR
      INTERFACE_NO_STR
      "Interface's name\n")
{
  vty->node = CONFIG_NODE;
  static char ifnumber[MAX_IFNAME_LENGTH];
  char ifnumber_temp[MAX_IFNAME_LENGTH] = {0};

  if (strchr(argv[0], '.'))
  {
     delete_sub_intf(argv[0]);
     return CMD_SUCCESS;
  }

  if (strstr(argv[0], LAG_PORT_NAME_PREFIX)) {
      vty_out(vty, "%% Could not remove %s, "
                   "use \'no interface %s %s\' %s",
                   argv[0],
                   LAG_PORT_NAME_PREFIX,
                   argv[0] + LAG_PORT_NAME_PREFIX_LENGTH,
                   VTY_NEWLINE);
      return CMD_WARNING;
  }

  if (VERIFY_VLAN_IFNAME(argv[0]) == 0) {
      GET_VLANIF(ifnumber_temp, argv[0]);
      if (delete_vlan_interface(ifnumber_temp) == CMD_OVSDB_FAILURE) {
          return CMD_OVSDB_FAILURE;
      }
  }
  else if (strlen(argv[0]) < MAX_IFNAME_LENGTH)
  {
    strncpy(ifnumber_temp, argv[0], MAX_IFNAME_LENGTH);
  }
  else
  {
    return CMD_ERR_NO_MATCH;
  }
  strncpy(ifnumber, ifnumber_temp, MAX_IFNAME_LENGTH);
  vty->index = ifnumber;
  return CMD_SUCCESS;
}

/*
 * Interface Diagnostics Information
 */
int cli_show_xvr_dom_exec (struct cmd_element *self, struct vty *vty,
        int flags, int argc, const char *argv[])
{
    const struct ovsrec_interface *ifrow = NULL;
    const char *cur_state =NULL;
    struct shash sorted_interfaces;
    const struct shash_node **nodes;
    int idx, count;
    bool validIntf = false;
    bool dom_info_present = false;
    bool is_qsfp_splittable = false;

    struct string_pairs
    {
        char *key;
        char *string;
        char *unit;
    };

    static struct string_pairs dom_keys [] = {
        {"temperature", "Temperature", "C" },
        {"temperature_high_alarm", "Temperature high alarm", "" },
        {"temperature_low_alarm", "Temperature low alarm", "" },
        {"temperature_high_warning", "Temperature high warning", "" },
        {"temperature_low_warning", "Temperature low warning", "" },
        {"temperature_high_alarm_threshold", "Temperature high alarm threshold", "C" },
        {"temperature_low_alarm_threshold", "Temperature low alarm threshold", "C" },
        {"temperature_high_warning_threshold", "Temperature high warning threshold", "C" },
        {"temperature_low_warning_threshold", "Temperature low warning threshold", "C" },
        {"vcc", "Voltage", "V" },
        {"vcc_high_alarm", "Voltage high alarm", "" },
        {"vcc_low_alarm", "Voltage high alarm", "" },
        {"vcc_high_warning", "Voltage high alarm", "" },
        {"vcc_low_warning", "Voltage low warning", "" },
        {"vcc_high_alarm_threshold", "Voltage high alarm threshold", "V" },
        {"vcc_low_alarm_threshold", "Voltage low alarm threshold", "V" },
        {"vcc_high_warning_threshold", "Voltage high warning threshold", "V" },
        {"vcc_low_warning_threshold", "Voltage low warning threshold", "V" },
        {"tx_bias", "Bias current", "mA" },
        {"tx_bias_high_alarm", "Bias current high alarm", "" },
        {"tx_bias_low_alarm", "Bias current low alarm", "" },
        {"tx_bias_high_warning", "Bias current high warning", "" },
        {"tx_bias_low_warning", "Bias current low warning", "" },
        {"tx_bias_high_alarm_threshold", "Bias current high alarm threshold", "mA" },
        {"tx_bias_low_alarm_threshold", "Bias current low alarm threshold", "mA" },
        {"tx_bias_high_warning_threshold", "Bias current high warning threshold", "mA" },
        {"tx_bias_low_warning_threshold", "Bias current low warning threshold", "mA" },
        {"rx_power", "Rx power", "mW" },
        {"rx_power_high_alarm", "Rx power high alarm", "" },
        {"rx_power_low_alarm", "Rx power low alarm", "" },
        {"rx_power_high_warning", "Rx power high warning", "" },
        {"rx_power_low_warning", "Rx power low warning", "" },
        {"rx_power_high_alarm_threshold", "Rx power high alarm threshold", "mW" },
        {"rx_power_low_alarm_threshold", "Rx power low alarm threshold", "mW" },
        {"rx_power_high_warning_threshold", "Rx power high warning threshold", "mW" },
        {"rx_power_low_warning_threshold", "Rx power low warning threshold", "mW" },
        {"tx_power", "Tx power", "mW" },
        {"tx_power_high_alarm", "Tx power high alarm", "" },
        {"tx_power_low_alarm", "Tx power low alarm", "" },
        {"tx_power_high_warning", "Tx power high warning", "" },
        {"tx_power_low_warning", "Tx power low warning", "" },
        {"tx_power_high_alarm_threshold", "Tx power high alarm threshold", "mW" },
        {"tx_power_low_alarm_threshold", "Tx power low alarm threshold", "mW" },
        {"tx_power_high_warning_threshold", "Tx power high warning threshold", "mW" },
        {"tx_power_low_warning_threshold", "Tx power low warning threshold", "mW" },
        {"-","\n Lane 1:\n", "" },
        {"tx1_bias", "Bias current", "mA" },
        {"tx1_bias_high_alarm", "Bias current high alarm", "" },
        {"tx1_bias_low_alarm", "Bias current low alarm", "" },
        {"tx1_bias_high_warning", "Bias current high warning", "" },
        {"tx1_bias_low_warning", "Bias current low warning", "" },
        {"tx1_bias_high_alarm_threshold", "Bias current high alarm threshold", "mA" },
        {"tx1_bias_low_alarm_threshold", "Bias current low alarm threshold", "mA" },
        {"tx1_bias_high_warning_threshold", "Bias current high warning threshold", "mA" },
        {"tx1_bias_low_warning_threshold", "Bias current low warning threshold", "mA" },
        {"rx1_power", "Rx power", "mW" },
        {"rx1_power_high_alarm", "Rx power high alarm", "" },
        {"rx1_power_low_alarm", "Rx power low alarm", "" },
        {"rx1_power_high_warning", "Rx power high warning", "" },
        {"rx1_power_low_warning", "Rx power low warning", "" },
        {"rx1_power_high_alarm_threshold", "Rx power high alarm threshold", "mW" },
        {"rx1_power_low_alarm_threshold", "Rx power low alarm threshold", "mW" },
        {"rx1_power_high_warning_threshold", "Rx power high warning threshold", "mW" },
        {"rx1_power_low_warning_threshold", "Rx power low warning threshold", "mW" },
        {"-","\n Lane 2:\n", "" },
        {"tx2_bias", "Bias current", "mA" },
        {"tx2_bias_high_alarm", "Bias current high alarm", "" },
        {"tx2_bias_low_alarm", "Bias current low alarm", "" },
        {"tx2_bias_high_warning", "Bias current high warning", "" },
        {"tx2_bias_low_warning", "Bias current low warning", "" },
        {"tx2_bias_high_alarm_threshold", "Bias current high alarm threshold", "mA" },
        {"tx2_bias_low_alarm_threshold", "Bias current low alarm threshold", "mA" },
        {"tx2_bias_high_warning_threshold", "Bias current high warning threshold", "mA" },
        {"tx2_bias_low_warning_threshold", "Bias current low warning threshold", "mA" },
        {"rx2_power", "Rx power", "mW" },
        {"rx2_power_high_alarm", "Rx power high alarm", "" },
        {"rx2_power_low_alarm", "Rx power low alarm", "" },
        {"rx2_power_high_warning", "Rx power high warning", "" },
        {"rx2_power_low_warning", "Rx power low warning", "" },
        {"rx2_power_high_alarm_threshold", "Rx power high alarm threshold", "mW" },
        {"rx2_power_low_alarm_threshold", "Rx power low alarm threshold", "mW" },
        {"rx2_power_high_warning_threshold", "Rx power high warning threshold", "mW" },
        {"rx2_power_low_warning_threshold", "Rx power low warning threshold", "mW" },
        {"-","\n Lane 3:\n", "" },
        {"tx3_bias", "Bias current", "mA" },
        {"tx3_bias_high_alarm", "Bias current high alarm", "" },
        {"tx3_bias_low_alarm", "Bias current low alarm", "" },
        {"tx3_bias_high_warning", "Bias current high warning", "" },
        {"tx3_bias_low_warning", "Bias current low warning", "" },
        {"tx3_bias_high_alarm_threshold", "Bias current high alarm threshold", "mA" },
        {"tx3_bias_low_alarm_threshold", "Bias current low alarm threshold", "mA" },
        {"tx3_bias_high_warning_threshold", "Bias current high warning threshold", "mA" },
        {"tx3_bias_low_warning_threshold", "Bias current low warning threshold", "mA" },
        {"rx3_power", "Rx power", "mW" },
        {"rx3_power_high_alarm", "Rx power high alarm", "" },
        {"rx3_power_low_alarm", "Rx power low alarm", "" },
        {"rx3_power_high_warning", "Rx power high warning", "" },
        {"rx3_power_low_warning", "Rx power low warning", "" },
        {"rx3_power_high_alarm_threshold", "Rx power high alarm threshold", "mW" },
        {"rx3_power_low_alarm_threshold", "Rx power low alarm threshold", "mW" },
        {"rx3_power_high_warning_threshold", "Rx power high warning threshold", "mW" },
        {"rx3_power_low_warning_threshold", "Rx power low warning threshold", "mW" },
        {"-","\n Lane 4:\n", "" },
        {"tx4_bias", "Bias current", "mA" },
        {"tx4_bias_high_alarm", "Bias current high alarm", "" },
        {"tx4_bias_low_alarm", "Bias current low alarm", "" },
        {"tx4_bias_high_warning", "Bias current high warning", "" },
        {"tx4_bias_low_warning", "Bias current low warning", "" },
        {"tx4_bias_high_alarm_threshold", "Bias current high alarm threshold", "mA" },
        {"tx4_bias_low_alarm_threshold", "Bias current low alarm threshold", "mA" },
        {"tx4_bias_high_warning_threshold", "Bias current high warning threshold", "mA" },
        {"tx4_bias_low_warning_threshold", "Bias current low warning threshold", "mA" },
        {"rx4_power", "Rx power", "mW" },
        {"rx4_power_high_alarm", "Rx power high alarm", "" },
        {"rx4_power_low_alarm", "Rx power low alarm", "" },
        {"rx4_power_high_warning", "Rx power high warning", "" },
        {"rx4_power_low_warning", "Rx power low warning", "" },
        {"rx4_power_high_alarm_threshold", "Rx power high alarm threshold", "mW" },
        {"rx4_power_low_alarm_threshold", "Rx power low alarm threshold", "mW" },
        {"rx4_power_high_warning_threshold", "Rx power high warning threshold", "mW" },
        {"rx4_power_low_warning_threshold", "Rx power low warning threshold", "mW" },
        { NULL, NULL, NULL }
    };

    shash_init(&sorted_interfaces);

    OVSREC_INTERFACE_FOR_EACH(ifrow, idl)
    {
        shash_add(&sorted_interfaces, ifrow->name, (void *)ifrow);
    }

    nodes = sort_interface(&sorted_interfaces);

    count = shash_count(&sorted_interfaces);

    for (idx = 0; idx < count; idx++)
    {
        dom_info_present = false;
        is_qsfp_splittable = false;

        ifrow = (const struct ovsrec_interface *)nodes[idx]->data;

        if ((NULL != argv[0]) && (0 != strcmp(argv[0],ifrow->name)))
        {
            continue;
        }

        if (strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_INTERNAL) == 0)
        {
            /* Skipping internal interfaces */
            continue;
        }
        validIntf = true;

        int i;

        /* Display transceiver information */
        vty_out (vty, "Interface %s:%s", ifrow->name, VTY_NEWLINE);

        if (strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_VLANSUBINT) == 0)
        {
            vty_out(vty, "Not supported by subinterface%s", VTY_NEWLINE);
        }
        cur_state = smap_get(&ifrow->hw_intf_info,
                             INTERFACE_HW_INTF_INFO_MAP_CONNECTOR);
        if (NULL != cur_state)
        {
            if (strcmp(cur_state,
                       INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_RJ45) == 0)
            {
                vty_out(vty, " Connector: RJ45%s", VTY_NEWLINE);
            }
            else if (strcmp(cur_state,
                            INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_SFP_PLUS) == 0)
            {
                vty_out(vty, " Connector: SFP+%s", VTY_NEWLINE);
            }
            else if (strcmp(cur_state,
                            INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_QSFP_PLUS) == 0)
            {
                cur_state = smap_get(&ifrow->hw_intf_info,
                                     INTERFACE_HW_INTF_INFO_MAP_SPLIT_4);
                if (cur_state != NULL &&
                        strcmp(cur_state,
                               INTERFACE_HW_INTF_INFO_MAP_SPLIT_4_TRUE) == 0)
                {
                    vty_out(vty, " Connector: QSFP+ (splittable)%s",
                            VTY_NEWLINE);
                    is_qsfp_splittable = true;
                }
                else
                {
                    vty_out(vty, " Connector: QSFP+ %s", VTY_NEWLINE);
                }
            }
            else if (strcmp(cur_state,
                            INTERFACE_HW_INTF_INFO_MAP_CONNECTOR_QSFP28) == 0)
            {
                cur_state = smap_get(&ifrow->hw_intf_info,
                                     INTERFACE_HW_INTF_INFO_MAP_SPLIT_4);
                if (cur_state != NULL &&
                        strcmp(cur_state,
                               INTERFACE_HW_INTF_INFO_MAP_SPLIT_4_TRUE) == 0)
                {
                    vty_out(vty, " Connector: QSFP28 (splittable)%s",
                            VTY_NEWLINE);
                    is_qsfp_splittable = true;
                }
                else
                {
                    vty_out(vty, " Connector: QSFP28 %s", VTY_NEWLINE);
                }
            }
            else
            {
                vty_out(vty, " Connector: %s%s", cur_state, VTY_NEWLINE);
            }
        }

        cur_state = smap_get(&ifrow->hw_intf_info,
                             INTERFACE_HW_INTF_INFO_MAP_PLUGGABLE);

        if (cur_state != NULL &&
            strcmp(cur_state, INTERFACE_HW_INTF_INFO_MAP_PLUGGABLE_TRUE) == 0)
        {
            cur_state = smap_get(&ifrow->pm_info,
                                 INTERFACE_PM_INFO_MAP_CONNECTOR);
            if (cur_state != NULL)
            {
                if (strcmp(cur_state,
                           OVSREC_INTERFACE_PM_INFO_CONNECTOR_ABSENT) == 0)
                {
                    vty_out(vty, " Transceiver module: not present%s",
                            VTY_NEWLINE);
                }
                else
                {
                    vty_out(vty, " Transceiver module: %s%s", cur_state,
                            VTY_NEWLINE);
                    for (i = 0; dom_keys[i].key != NULL; i++)
                    {
                        cur_state = smap_get(&ifrow->pm_info,
                                             dom_keys[i].key);
                        if (cur_state != NULL)
                        {
                            vty_out(vty, "  %s: ", dom_keys[i].string);
                            vty_out(vty, "%s%s%s", cur_state, dom_keys[i].unit, VTY_NEWLINE);
                            dom_info_present = true;
                        }
                        if (dom_info_present == true && is_qsfp_splittable == true &&
                            !strcmp(dom_keys[i].key, "-")) {
                            vty_out(vty, "%s", dom_keys[i].string);
                        }
                    }
                    if (dom_info_present == false)
                        vty_out(vty, " %% No DOM information available%s",
                                VTY_NEWLINE);

                }
            }
            else
            {
                vty_out(vty, " Transceiver module: no information available%s",
                        VTY_NEWLINE);
            }
        }
        vty_out (vty, "%s", VTY_NEWLINE);
    }

    shash_destroy(&sorted_interfaces);
    free(nodes);

    if (validIntf)
    {
        return CMD_SUCCESS;
    }
    else
    {
        vty_out (vty, "Invalid switch interface ID.%s", VTY_NEWLINE);
        return CMD_OVSDB_FAILURE;
    }
}

DEFUN (cli_intf_show_interface_ifname_dom,
        cli_intf_show_interface_ifname_dom_cmd,
        "show interface IFNAME dom",
        SHOW_STR
        INTERFACE_STR
        IFNAME_STR
        "Show transceiver diagnostics info for interface\n")
{
    return cli_show_xvr_dom_exec (self, vty, vty_flags, argc, argv);
}

DEFUN (cli_intf_show_interface_dom,
        cli_intf_show_interface_dom_cmd,
        "show interface dom",
        SHOW_STR
        INTERFACE_STR
        "Show transceiver diagnostics info for interfaces\n")
{
    argv[0] = NULL;
    return cli_show_xvr_dom_exec (self, vty, vty_flags, argc, argv);
}


DEFUN (cli_intf_show_intferface_ifname,
        cli_intf_show_intferface_ifname_cmd,
        "show interface IFNAME {brief|transceiver|queues}",
        SHOW_STR
        INTERFACE_STR
        IFNAME_STR
        "Show brief info of interface\n"
        "Show transceiver info for interface\n"
        "Show tx queue info for interface\n")
{
    bool brief = false;
    bool transceiver = false;
    int rc = CMD_SUCCESS;

    if ((NULL != argv[1]) && (strcmp(argv[1], "brief") == 0))
    {
        brief = true;
    }
    if ((NULL != argv[2]) && (strcmp(argv[2], "transceiver") == 0))
    {
        transceiver = true;
    }

    if ((NULL != argv[3]) && (strcmp(argv[3], "queues") == 0))
    {
        return (cli_show_interface_queue_stats (self, vty, argc, argv));
    }

    if (transceiver)
    {
        rc = cli_show_xvr_exec (self, vty, vty_flags, argc, argv, brief);
    }
    else
    {
        rc = cli_show_interface_exec (self, vty, vty_flags, argc, argv, brief);
    }

    return rc;
}

DEFUN (cli_intf_show_intferface_ifname_br,
        cli_intf_show_intferface_ifname_br_cmd,
        "show interface {brief|transceiver|queues}",
        SHOW_STR
        INTERFACE_STR
        "Show brief info of interfaces\n"
        "Show transceiver info for interfaces\n"
        "Show tx queue info for interfaces\n")
{
    bool brief = false;
    bool transceiver = false;
    int rc = CMD_SUCCESS;

    if ((NULL != argv[0]) && (strcmp(argv[0], "brief") == 0))
    {
        brief = true;
    }
    if ((NULL != argv[1]) && (strcmp(argv[1], "transceiver") == 0))
    {
        transceiver = true;
    }

    if ((NULL != argv[2]) && (strcmp(argv[2], "queues") == 0))
    {
        return (cli_show_interface_queue_stats (self, vty, argc, argv));
    }

    argv[0] = NULL;

    if (transceiver)
    {
        rc = cli_show_xvr_exec (self, vty, vty_flags, argc, argv, brief);
    }
    else
    {
        rc = cli_show_interface_exec (self, vty, vty_flags, argc, argv, brief);
    }
    return rc;
}

static void
show_ip_stats(struct vty *vty, bool isIpv6, const struct ovsdb_datum *datum)
{
    char *ipv4_interface_statistics_keys [] = {
        "ipv4_uc_rx_packets",
        "ipv4_uc_rx_bytes",
        "ipv4_mc_rx_packets",
        "ipv4_mc_rx_bytes",
        "ipv4_uc_tx_packets",
        "ipv4_uc_tx_bytes",
        "ipv4_mc_tx_packets",
        "ipv4_mc_tx_bytes"
    };

    char *ipv6_interface_statistics_keys [] = {
        "ipv6_uc_rx_packets",
        "ipv6_uc_rx_bytes",
        "ipv6_mc_rx_packets",
        "ipv6_mc_rx_bytes",
        "ipv6_uc_tx_packets",
        "ipv6_uc_tx_bytes",
        "ipv6_mc_tx_packets",
        "ipv6_mc_tx_bytes"
    };

    unsigned int index;
    union ovsdb_atom atom;
    char **stat_keys = (isIpv6 == true) ? ipv6_interface_statistics_keys :
        ipv4_interface_statistics_keys;

    vty_out(vty, " RX%s", VTY_NEWLINE);

    atom.string = stat_keys[0];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "%10sucast: %"PRIu64" packets, ", "",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    atom.string = stat_keys[1];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "%"PRIu64" bytes",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    vty_out(vty, "%s", VTY_NEWLINE);

    atom.string = stat_keys[2];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "%10smcast: %"PRIu64" packets, ", "",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    atom.string = stat_keys[3];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "%"PRIu64" bytes",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    vty_out(vty, "%s", VTY_NEWLINE);

    vty_out(vty, " TX%s", VTY_NEWLINE);

    atom.string = stat_keys[4];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "%10sucast: %"PRIu64" packets, ", "",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    atom.string = stat_keys[5];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "%"PRIu64" bytes",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    vty_out(vty, "%s", VTY_NEWLINE);

    atom.string = stat_keys[6];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "%10smcast: %"PRIu64" packets, ", "",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    atom.string = stat_keys[7];
    index = ovsdb_datum_find_key(datum, &atom, OVSDB_TYPE_STRING);
    vty_out(vty, "%"PRIu64" bytes",
            (index == UINT_MAX)? 0 : datum->values[index].integer);
    vty_out(vty, "%s", VTY_NEWLINE);
}

static void cli_show_ip_lag(const char *if_name, struct vty *vty, const char *argv[])
{
    const struct ovsrec_port *port_row = NULL;
    bool isIpv6 = false;
    int i;
    bool forwarding_state;
    bool port_aggregation_forwarding;

    if(!strncmp(argv[0], "ipv6", IPV6_LENGTH)) {
        isIpv6 = true;
    }

    OVSREC_PORT_FOR_EACH(port_row, idl) {
        if (if_name &&
            strncmp(port_row->name, if_name, INTF_NAME_SIZE) == 0 &&
            strncmp(port_row->name, LAG_PORT_NAME_PREFIX, LAG_PORT_NAME_PREFIX_LENGTH) == 0) {
            if (!check_port_in_vrf(port_row->name)) {
                vty_out (vty, "Interface %s is not L3.%s", if_name, VTY_NEWLINE);
                break;
            }
            else {
                vty_out (vty, "Aggregate-name %s %s", port_row->name, VTY_NEWLINE);

                forwarding_state = smap_get_bool(&port_row->forwarding_state,
                                                 PORT_FORWARDING_STATE_MAP_FORWARDING,
                                                 false);
                vty_out (vty, "Forwarding State : %s %s",
                         forwarding_state? "forwarding":"not forwarding", VTY_NEWLINE);

                port_aggregation_forwarding =
                        smap_get_bool(&port_row->forwarding_state,
                                      PORT_FORWARDING_STATE_MAP_PORT_AGGREGATION_FORWARDING,
                                      false);
                vty_out (vty, "Port aggregation forwarding : %s %s",
                         port_aggregation_forwarding? "forwarding":"not forwarding",
                         VTY_NEWLINE);

                /* Displaying primary and secondary addresses*/
                if(isIpv6)
                {
                    if (port_row->ip6_address) {
                        vty_out(vty, "IPv6 address %s%s", port_row->ip6_address,
                                VTY_NEWLINE);
                    }
                    for (i = 0; i < port_row->n_ip6_address_secondary; i++) {
                        vty_out(vty, "IPv6 address %s secondary%s",
                                port_row->ip6_address_secondary[i],
                                VTY_NEWLINE);
                    }

                }
                else
                {
                    if (port_row->ip4_address) {
                        vty_out(vty, "IPv4 address %s%s", port_row->ip4_address,
                                VTY_NEWLINE);
                    }
                    for (i = 0; i < port_row->n_ip4_address_secondary; i++) {
                        vty_out(vty, "IPv4 address %s secondary%s",
                                port_row->ip4_address_secondary[i],
                                VTY_NEWLINE);
                    }
                }
            }
        }
    }
}

static int
cli_show_ip_interface_exec(const char *argv[], int argc,
                           struct vty *vty)
{
    const struct ovsrec_interface *ifrow = NULL;
    const struct ovsrec_interface *if_parent_row = NULL;
    const struct ovsrec_port *port_row = NULL;
    const struct ovsdb_datum *datum;
    struct shash sorted_interfaces;
    const struct shash_node **nodes;
    int idx, count, i;
    int64_t intVal = 0;
    const char *if_name = NULL;
    bool internal_if = false;
    bool isIpv6 = false;
    if(!strcmp(argv[0], "ipv6"))
        isIpv6 = true;
    int64_t key_subintf_parent = 0;

    if(argc > 1 && NULL != argv[1])
        if_name = argv[1];

    shash_init(&sorted_interfaces);

    OVSREC_INTERFACE_FOR_EACH(ifrow, idl)
    {
        shash_add(&sorted_interfaces, ifrow->name, (void *)ifrow);
    }

    nodes = sort_interface(&sorted_interfaces);

    count = shash_count(&sorted_interfaces);

    for (idx = 0; idx < count; idx++)
    {
        ifrow = (const struct ovsrec_interface *)nodes[idx]->data;

        if (if_name && (0 != strcmp(if_name, ifrow->name)))
            continue;

        if (!check_iface_in_vrf(ifrow->name) || !(port_row = port_find(ifrow->name)))
        {
            if(if_name)
            {
                vty_out (vty, "Interface %s is not L3.%s", if_name, VTY_NEWLINE);
                return 0;
            }
            continue;
        }
        if (isIpv6)
        {
            if(!((port_row->ip6_address)))
            continue;
        }
        else if (!((port_row->ip4_address)))
            continue;

        intVal = 0;
        vty_out(vty, "%s", VTY_NEWLINE);
        internal_if = (strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_INTERNAL) == 0) ? true : false;

        if(strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_VLANSUBINT) == 0)
        {
            if (ifrow->n_subintf_parent > 0)
            {
                if_parent_row = ifrow->value_subintf_parent[0];
            }

            if (ifrow->n_subintf_parent > 0)
            {
                key_subintf_parent = ifrow->key_subintf_parent[0];
            }

            show_subinterface_status(ifrow, false, if_parent_row, key_subintf_parent);
            vty_out (vty, " Hardware: Ethernet, MAC Address: %s %s",
                    if_parent_row->mac_in_use, VTY_NEWLINE);
        }
        else if(strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_LOOPBACK) == 0)
        {
            show_interface_status(vty, ifrow, true, false);
            vty_out (vty, " Hardware: Loopback %s", VTY_NEWLINE);
        }
        else
        {
            show_interface_status(vty, ifrow, internal_if, false);
            vty_out (vty, " Hardware: Ethernet, MAC Address: %s %s",
                    ifrow->mac_in_use, VTY_NEWLINE);
        }

        /* Displaying primary and secondary addresses*/
        if(isIpv6)
        {
            if (port_row->ip6_address) {
                vty_out(vty, " IPv6 address %s%s", port_row->ip6_address,
                        VTY_NEWLINE);
            }
            for (i = 0; i < port_row->n_ip6_address_secondary; i++) {
                vty_out(vty, " IPv6 address %s secondary%s",
                        port_row->ip6_address_secondary[i],
                        VTY_NEWLINE);
            }

        }
        else
        {
            if (port_row->ip4_address) {
                vty_out(vty, " IPv4 address %s%s", port_row->ip4_address,
                        VTY_NEWLINE);
            }
            for (i = 0; i < port_row->n_ip4_address_secondary; i++) {
                vty_out(vty, " IPv4 address %s secondary%s",
                        port_row->ip4_address_secondary[i],
                        VTY_NEWLINE);
            }
        }

        datum = ovsrec_interface_get_mtu(ifrow, OVSDB_TYPE_INTEGER);
        if ((NULL!=datum) && (datum->n >0))
        {
            intVal = datum->keys[0].integer;
        }

        if(strcmp(ifrow->type, "system") != 0)
        {
            if(if_name && !strcmp(if_name, ifrow->name))
            {
                return 0;
            }
            continue;
        }

        vty_out(vty, " MTU %ld %s", intVal, VTY_NEWLINE);

        datum = ovsrec_interface_get_statistics(ifrow,
                OVSDB_TYPE_STRING, OVSDB_TYPE_INTEGER);

        if (NULL==datum) continue;

        show_ip_stats(vty, isIpv6, datum);

        if (if_name)
        {
            break;
        }
    }
    //Check if this is a LAG interface
    if (idx == count) {
        cli_show_ip_lag(if_name, vty, argv);
    }

    return 0;
}

DEFUN (cli_intf_show_ip_intferface,
        cli_intf_show_ip_intferface_ifname_cmd,
        "show (ip|ipv6) interface [IFNAME]",
        SHOW_STR
        IP_STR
        IPV6_STR
        INTERFACE_STR
        IFNAME_STR)
{
    cli_show_ip_interface_exec(argv, argc, vty);
    vty_out (vty, "%s", VTY_NEWLINE);
    return CMD_SUCCESS;
}

/*******************************************************************
 * @func        : tempd_ovsdb_init
 * @detail      : Add interface related table & columns to ops-cli
 *                idl cache
 *******************************************************************/
static void
intf_ovsdb_init(void)
{
    ovsdb_idl_add_table(idl, &ovsrec_table_interface);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_lldp_statistics);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_other_config);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_lldp_neighbor_info);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_user_config);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_link_state);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_admin_state);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_duplex);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_mtu);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_mac_in_use);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_link_speed);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_pause);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_statistics);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_queue_tx_bytes);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_queue_tx_packets);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_queue_tx_errors);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_type);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_hw_intf_info);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_pm_info);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_error);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_lacp_status);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_subintf_parent);
    ovsdb_idl_add_table(idl, &ovsrec_table_vrf);
    ovsdb_idl_add_column(idl, &ovsrec_vrf_col_ports);
    ovsdb_idl_add_table(idl, &ovsrec_table_port);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_qos_status);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_forwarding_state);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_ip4_address);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_ip4_address_secondary);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_ip6_address);
    ovsdb_idl_add_column(idl, &ovsrec_port_col_ip6_address_secondary);

    return;
}


/* Initialize ops-intfd cli node.
 */
void cli_pre_init(void)
{
    vtysh_ret_val retval = e_vtysh_error;

    install_node (&interface_node, NULL);
    vtysh_install_default (INTERFACE_NODE);
    install_dyn_helpstr_funcptr("dyncb_helpstr_1G", dyncb_helpstr_speeds);
    install_dyn_helpstr_funcptr("dyncb_helpstr_10G", dyncb_helpstr_speeds);
    install_dyn_helpstr_funcptr("dyncb_helpstr_25G", dyncb_helpstr_speeds);
    install_dyn_helpstr_funcptr("dyncb_helpstr_40G", dyncb_helpstr_speeds);
    install_dyn_helpstr_funcptr("dyncb_helpstr_50G", dyncb_helpstr_speeds);
    install_dyn_helpstr_funcptr("dyncb_helpstr_100G", dyncb_helpstr_speeds);
    install_dyn_helpstr_funcptr("dyncb_helpstr_mtu", dyncb_helpstr_mtu);

    intf_ovsdb_init();

    /* Initialize interface context show running client callback function. */
    retval = install_show_run_config_context(e_vtysh_interface_context,
                                &vtysh_intf_context_clientcallback,
                                &vtysh_intf_context_init, &vtysh_intf_context_exit);
    if(e_vtysh_ok != retval)
    {
        vtysh_ovsdb_config_logmsg(VTYSH_OVSDB_CONFIG_ERR,
                              "interface context unable to add config callback");
        assert(0);
    }
    return;
}


/* Initialize ops-intfd cli element.
 */
void cli_post_init(void)
{
    /* Config commands */
    install_element (INTERFACE_NODE, &cli_intf_shutdown_cmd);
    install_element (CONFIG_NODE, &vtysh_interface_cmd);
    install_element (INTERFACE_NODE, &no_cli_intf_shutdown_cmd);
    install_element (CONFIG_NODE, &no_vtysh_interface_cmd);
    install_element (INTERFACE_NODE, &vtysh_exit_interface_cmd);
    install_element (INTERFACE_NODE, &cli_intf_split_cmd);
    install_element (INTERFACE_NODE, &no_cli_intf_split_cmd);
    install_element (INTERFACE_NODE, &cli_intf_speed_cmd);
    install_element (INTERFACE_NODE, &no_cli_intf_speed_cmd);
    install_element (INTERFACE_NODE, &cli_intf_mtu_cmd);
    install_element (INTERFACE_NODE, &no_cli_intf_mtu_cmd);
    install_element (INTERFACE_NODE, &cli_intf_duplex_cmd);
    install_element (INTERFACE_NODE, &no_cli_intf_duplex_cmd);
    install_element (INTERFACE_NODE, &cli_intf_flowcontrol_cmd);
    install_element (INTERFACE_NODE, &no_cli_intf_flowcontrol_cmd);
    install_element (INTERFACE_NODE, &cli_intf_autoneg_cmd);
    install_element (INTERFACE_NODE, &no_cli_intf_autoneg_cmd);

    /* Show commands */
    install_element (ENABLE_NODE, &cli_intf_show_intferface_ifname_cmd);
    install_element (ENABLE_NODE, &cli_intf_show_intferface_ifname_br_cmd);
    install_element (ENABLE_NODE, &cli_intf_show_ip_intferface_ifname_cmd);
    install_element (ENABLE_NODE, &cli_intf_show_interface_dom_cmd);
    install_element (ENABLE_NODE, &cli_intf_show_interface_ifname_dom_cmd);
    install_element (ENABLE_NODE, &cli_intf_show_run_intf_cmd);
    install_element (ENABLE_NODE, &cli_intf_show_run_intf_if_cmd);
    install_element (ENABLE_NODE, &cli_intf_show_run_intf_mgmt_cmd);

    install_element (VLAN_INTERFACE_NODE, &cli_intf_shutdown_cmd);
    install_element (VLAN_INTERFACE_NODE, &no_cli_intf_shutdown_cmd);

    return;
}
