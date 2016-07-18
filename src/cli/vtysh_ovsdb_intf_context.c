/*
 * Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002 Kunihiro Ishiguro
 * Copyright (C) 2015 Hewlett Packard Enterprise Development LP
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
 */
/****************************************************************************
 * @ingroup cli
 *
 * @file vtysh_ovsdb_intf_context.c
 * Source for registering client callback with interface context.
 *
 ***************************************************************************/

#include "vtysh/zebra.h"
#include "vtysh/vty.h"
#include "vtysh/vector.h"
#include "vswitch-idl.h"
#include "openswitch-idl.h"
#include "qos_intf.h"
#include "vtysh/command.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "vtysh_ovsdb_intf_context.h"
#include "intf_vty.h"
#include "vtysh/utils/lacp_vtysh_utils.h"
#include "vtysh/utils/intf_vtysh_utils.h"
#include "vtysh/utils/vlan_vtysh_utils.h"

static struct shash sorted_interfaces;

typedef struct vtysh_ovsdb_intf_cfg_struct
{
  bool disp_intf_cfg;
} vtysh_ovsdb_intf_cfg;

#if 0
/*-----------------------------------------------------------------------------
| Function : intfd_get_user_cfg_adminstate
| Responsibility : get teh admin state form user_config column in specific row
| Parameters :
|   const struct smap *ifrow_config : specific row config
| Return : bool *adminstate : returns the admin state
-----------------------------------------------------------------------------*/
static void
intfd_get_user_cfg_adminstate(const struct smap *ifrow_config,
                              bool *adminstate)
{
  const char *data = NULL;

  data = smap_get(ifrow_config, INTERFACE_USER_CONFIG_MAP_ADMIN);

  if (data && (VTYSH_STR_EQ(data, OVSREC_INTERFACE_USER_CONFIG_ADMIN_UP)))
  {
    *adminstate = true;
  }
}
#endif

/*-----------------------------------------------------------------------------
| Function : is_parent_interface_split
| Responsibility : Check if parent interface has been split
| Parameters :
|   const struct ovsrec_interface *parent_iface : Parent Interface row data
|                                                 for the specific child
| Return : bool : returns true/false
-----------------------------------------------------------------------------*/
bool
is_parent_interface_split(const struct ovsrec_interface *parent_iface)
{
    const char *lanes_split_value = NULL;
    bool is_split = false;

    lanes_split_value = smap_get(&parent_iface->user_config,
                               INTERFACE_USER_CONFIG_MAP_LANE_SPLIT);
    if ((lanes_split_value != NULL) &&
        (strcmp(lanes_split_value,
                INTERFACE_USER_CONFIG_MAP_LANE_SPLIT_SPLIT) == 0))
      {
        /* Parent interface is split.
         * Display child interface configurations. */
        is_split = true;
      }
    return is_split;
}


#define PRINT_INT_HEADER_IN_SHOW_RUN if(!intfcfg.disp_intf_cfg) \
{ \
   intfcfg.disp_intf_cfg = true;\
   vtysh_ovsdb_cli_print(p_msg, "interface %s ", ifrow->name);\
}

struct feature_sorted_list *
vtysh_intf_context_init(void *p_private)
{
   vtysh_ovsdb_cbmsg_ptr p_msg = (vtysh_ovsdb_cbmsg *)p_private;
   const struct ovsrec_interface *ifrow;
   const struct shash_node **nodes;
   struct feature_sorted_list *sorted_list = NULL;
   int count;

   shash_init(&sorted_interfaces);

   OVSREC_INTERFACE_FOR_EACH(ifrow, p_msg->idl)
   {
       shash_add(&sorted_interfaces, ifrow->name, (void *)ifrow);
   }

   nodes = sort_interface(&sorted_interfaces);
   count = shash_count(&sorted_interfaces);
   sorted_list = (struct feature_sorted_list *)
                 malloc (sizeof(struct feature_sorted_list));
   if (sorted_list != NULL) {
       sorted_list->nodes = nodes;
       sorted_list->count = count;
   }

   return sorted_list;
}

vtysh_ret_val
vtysh_intf_context_clientcallback(void *p_private)
{
   vtysh_ovsdb_cbmsg_ptr p_msg = (vtysh_ovsdb_cbmsg *)p_private;
   const struct ovsrec_interface *ifrow = (struct ovsrec_interface *)p_msg->feature_row;
   const char *cur_state =NULL;
   int64_t vlan_number = 0;
   const struct ovsrec_port *port_row = NULL;
   struct smap other_config = SMAP_INITIALIZER(&other_config);

   vtysh_ovsdb_intf_cfg intfcfg;

   /* set to default values */
   intfcfg.disp_intf_cfg = false;

   if (ifrow && !strcmp(ifrow->name, DEFAULT_BRIDGE_NAME)) {
       p_msg->skip_subcontext_list = true;
       return e_vtysh_ok;
   }

   if (ifrow->split_parent != NULL &&
           !is_parent_interface_split(ifrow->split_parent)) {
       /* Parent is not split. Don't display child interfaces. */
       p_msg->skip_subcontext_list = true;
       return e_vtysh_ok;
   }

   /* TODO: Will be taken as part of sub-interface code modularization */
   if (strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_LOOPBACK) == 0)
   {
       vtysh_ovsdb_cli_print(p_msg, "interface loopback %s",
               ifrow->name + 8);
       p_msg->disp_header_cfg = true;
       return e_vtysh_ok;
   }
   if (strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_VLANSUBINT) == 0)
   {
       PRINT_INT_HEADER_IN_SHOW_RUN;
   }

   cur_state = smap_get(&ifrow->user_config, INTERFACE_USER_CONFIG_MAP_ADMIN);
   if ((NULL != cur_state)
         && (strcmp(cur_state, OVSREC_INTERFACE_USER_CONFIG_ADMIN_UP) == 0))
   {
      PRINT_INT_HEADER_IN_SHOW_RUN;
      vtysh_ovsdb_cli_print(p_msg, "%4s%s", "", "no shutdown");
   }

   /* sFlow config */
   if ((port_row = port_find(ifrow->name))!=NULL)
   {
       cur_state = smap_get(&port_row->other_config,
                            PORT_OTHER_CONFIG_SFLOW_PER_INTERFACE_KEY_STR);
       if ((NULL != cur_state) &&
           (strcmp(cur_state,
                   PORT_OTHER_CONFIG_SFLOW_PER_INTERFACE_VALUE_FALSE) == 0))
       {
          PRINT_INT_HEADER_IN_SHOW_RUN;
          show_sflow_config(ifrow->name, "    ", false);
       }
   }

   if (strcmp(ifrow->type, OVSREC_INTERFACE_TYPE_VLANSUBINT) == 0)
   {
       if (0 < ifrow->n_subintf_parent)
       {
           vlan_number = ifrow->key_subintf_parent[0];
       }
       if (0 != vlan_number)
       {
           vtysh_ovsdb_cli_print(p_msg, "%4s%s%d", "",
                   "encapsulation dot1Q ", vlan_number);
       }
   }

   cur_state = smap_get(&ifrow->user_config, INTERFACE_USER_CONFIG_MAP_LANE_SPLIT);

   if ((NULL != cur_state)
         && (strcmp(cur_state, INTERFACE_USER_CONFIG_MAP_LANE_SPLIT_SPLIT) == 0))
   {
       PRINT_INT_HEADER_IN_SHOW_RUN;
       vtysh_ovsdb_cli_print(p_msg, "%4s%s", "", "split");
   }

   cur_state = smap_get(&ifrow->user_config, INTERFACE_USER_CONFIG_MAP_SPEEDS);
   if ((NULL != cur_state)
         && (strcmp(cur_state, INTERFACE_USER_CONFIG_MAP_SPEEDS_DEFAULT) != 0))
   {
      PRINT_INT_HEADER_IN_SHOW_RUN;
      vtysh_ovsdb_cli_print(p_msg, "%4s%s %s", "", "speed", cur_state);
   }

   cur_state = smap_get(&ifrow->user_config, INTERFACE_USER_CONFIG_MAP_MTU);
   if ((NULL != cur_state)
         && (strcmp(cur_state, INTERFACE_USER_CONFIG_MAP_MTU_DEFAULT) != 0))
   {
      PRINT_INT_HEADER_IN_SHOW_RUN;
      vtysh_ovsdb_cli_print(p_msg, "%4s%s %s", "", "mtu", cur_state);
   }

   cur_state = smap_get(&ifrow->user_config, INTERFACE_USER_CONFIG_MAP_DUPLEX);
   if ((NULL != cur_state)
         && (strcmp(cur_state, INTERFACE_USER_CONFIG_MAP_DUPLEX_FULL) != 0))
   {
      PRINT_INT_HEADER_IN_SHOW_RUN;
      vtysh_ovsdb_cli_print(p_msg, "%4s%s %s", "", "duplex", cur_state);
   }


   cur_state = smap_get(&ifrow->user_config, INTERFACE_USER_CONFIG_MAP_PAUSE);
   if ((NULL != cur_state)
         && (strcmp(cur_state, INTERFACE_USER_CONFIG_MAP_PAUSE_NONE) != 0))
   {
      PRINT_INT_HEADER_IN_SHOW_RUN;
      if(strcmp(cur_state, INTERFACE_USER_CONFIG_MAP_PAUSE_RX) == 0)
      {
         vtysh_ovsdb_cli_print(p_msg, "%4s%s", "", "flowcontrol receive on");
      }
      else if(strcmp(cur_state, INTERFACE_USER_CONFIG_MAP_PAUSE_TX) == 0)
      {
         vtysh_ovsdb_cli_print(p_msg, "%4s%s", "", "flowcontrol send on");
      }
      else
      {
         vtysh_ovsdb_cli_print(p_msg, "%4s%s", "", "flowcontrol receive on");
         vtysh_ovsdb_cli_print(p_msg, "%4s%s", "", "flowcontrol send on");
      }
   }

   cur_state = smap_get(&ifrow->user_config, INTERFACE_USER_CONFIG_MAP_AUTONEG);
   if ((NULL != cur_state)
         && (strcmp(cur_state, INTERFACE_USER_CONFIG_MAP_AUTONEG_DEFAULT) != 0)
         && (strcmp(cur_state, INTERFACE_USER_CONFIG_MAP_AUTONEG_ON) != 0))
   {
      PRINT_INT_HEADER_IN_SHOW_RUN;
      vtysh_ovsdb_cli_print(p_msg, "%4s%s %s", "", "autonegotiation", cur_state);
   }

   const struct ovsrec_port *qos_port_row = port_lookup(ifrow->name, p_msg->idl);
   qos_trust_port_show_running_config(qos_port_row, &intfcfg.disp_intf_cfg, "interface");
   qos_apply_port_show_running_config(qos_port_row, &intfcfg.disp_intf_cfg, "interface");
   qos_cos_port_show_running_config(qos_port_row, &intfcfg.disp_intf_cfg, "interface");
   qos_dscp_port_show_running_config(qos_port_row, &intfcfg.disp_intf_cfg, "interface");

   p_msg->disp_header_cfg = intfcfg.disp_intf_cfg;

   return e_vtysh_ok;
}

void
vtysh_intf_context_exit(struct feature_sorted_list *list)
{
   shash_destroy(&sorted_interfaces);
   free(list->nodes);
   free(list);
}
