/*
 Copyright (C) 2015 Hewlett Packard Enterprise Development LP
 All Rights Reserved.

 Licensed under the Apache License, Version 2.0 (the "License"); you may
 not use this file except in compliance with the License. You may obtain
 a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 License for the specific language governing permissions and limitations
 under the License.
*/
/************************************************************************//**
 * @ingroup ops-intfd
 *
 * @file
 * Header for ops-intfd utility functions.
 *
 ***************************************************************************/

#ifndef __INTFD_UTILS_H__
#define __INTFD_UTILS_H__

/** @ingroup ops-intfd
 * @{ */

extern void intfd_print_smap(const char *name, const struct smap *map);

extern const char* intfd_get_error_str(enum ovsrec_interface_error_e reason);
extern const char* intfd_get_intf_type_str(enum ovsrec_interface_hw_intf_config_interface_type_e intf_type);
extern const char* intfd_get_lane_split_str(enum ovsrec_interface_user_config_lane_split_e ls);

/** @} end of group ops-intfd */

#endif /* __INTFD_UTILS_H__ */
