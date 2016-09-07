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
 * Source for intfd utility functions.
 *
 ***************************************************************************/

#include <smap.h>
#include <openvswitch/vlog.h>

#include <openswitch-idl.h>
#include <vswitch-idl.h>

VLOG_DEFINE_THIS_MODULE(intfd_utils);

/** @ingroup intfd
 * @{ */

void
intfd_print_smap(const char *name, const struct smap *map)
{
    struct smap_node *node;

    VLOG_DBG("intfd - %s", name);

    SMAP_FOR_EACH(node, map) {
        VLOG_DBG("\t%s=%s", node->key, node->value);
    }
} /* intfd_print_smap */

const char*
intfd_get_lane_split_str(enum ovsrec_interface_user_config_lane_split_e ls)
{
    switch(ls) {
    case INTERFACE_USER_CONFIG_LANE_SPLIT_NO_SPLIT:
        return INTERFACE_USER_CONFIG_MAP_LANE_SPLIT_NO_SPLIT;

    case INTERFACE_USER_CONFIG_LANE_SPLIT_SPLIT:
        return INTERFACE_USER_CONFIG_MAP_LANE_SPLIT_SPLIT;

    case INTERFACE_USER_CONFIG_LANE_SPLIT_DEFAULT:
    default:
        return "unset by user";
    }
} /* intfd_get_lane_split_str */

const char*
intfd_get_error_str(enum ovsrec_interface_error_e reason)
{
    switch(reason) {

    case INTERFACE_ERROR_UNINITIALIZED:
        return OVSREC_INTERFACE_ERROR_UNINITIALIZED;

    case INTERFACE_ERROR_ADMIN_DOWN:
        return OVSREC_INTERFACE_ERROR_ADMIN_DOWN;

    case INTERFACE_ERROR_MODULE_MISSING:
        return OVSREC_INTERFACE_ERROR_MODULE_MISSING;

    case INTERFACE_ERROR_MODULE_UNRECOGNIZED:
        return OVSREC_INTERFACE_ERROR_MODULE_UNRECOGNIZED;

    case INTERFACE_ERROR_MODULE_UNSUPPORTED:
        return OVSREC_INTERFACE_ERROR_MODULE_UNSUPPORTED;

    case INTERFACE_ERROR_LANES_SPLIT:
        return OVSREC_INTERFACE_ERROR_LANES_SPLIT;

    case INTERFACE_ERROR_LANES_NOT_SPLIT:
        return OVSREC_INTERFACE_ERROR_LANES_NOT_SPLIT;

    case INTERFACE_ERROR_INVALID_MTU:
        return OVSREC_INTERFACE_ERROR_INVALID_MTU;

    case INTERFACE_ERROR_INVALID_SPEEDS:
        return OVSREC_INTERFACE_ERROR_INVALID_SPEEDS;

    case INTERFACE_ERROR_AUTONEG_NOT_SUPPORTED:
        return OVSREC_INTERFACE_ERROR_AUTONEG_NOT_SUPPORTED;

    case INTERFACE_ERROR_AUTONEG_REQUIRED:
        return OVSREC_INTERFACE_ERROR_AUTONEG_REQUIRED;

    case PORT_ERROR_ADMIN_DOWN:
        return OVSREC_PORT_ERROR_ADMIN_DOWN;

    case INTERFACE_ERROR_OK:
        return OVSREC_INTERFACE_ERROR_OK;
    default:
        return "???";
    }

} /* intfd_get_error_str */

const char*
intfd_get_intf_type_str(enum ovsrec_interface_hw_intf_config_interface_type_e intf_type)
{
    switch(intf_type) {

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_UNKNOWN:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_UNKNOWN;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_BACKPLANE:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_BACKPLANE;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_1GBASE_SX:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_1GBASE_SX;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_1GBASE_T:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_1GBASE_T;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_10GBASE_CR:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_10GBASE_CR;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_10GBASE_SR:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_10GBASE_SR;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_10GBASE_LR:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_10GBASE_LR;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_10GBASE_LRM:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_10GBASE_LRM;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_10GBASE_ER:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_10GBASE_ER;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_25GBASE_CR:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_25GBASE_CR;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_25GBASE_SR:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_25GBASE_SR;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_25GBASE_LR:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_25GBASE_LR;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_40GBASE_CR4:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_40GBASE_CR4;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_40GBASE_SR4:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_40GBASE_SR4;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_40GBASE_LR4:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_40GBASE_LR4;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_100GBASE_CR4:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_100GBASE_CR4;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_100GBASE_SR4:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_100GBASE_SR4;

    case INTERFACE_HW_INTF_CONFIG_INTERFACE_TYPE_100GBASE_LR4:
        return INTERFACE_HW_INTF_CONFIG_MAP_INTERFACE_TYPE_100GBASE_LR4;

    default:
        return "unknown";
    }

} /* intfd_get_intf_type_str */

/** @} end of group intfd */
