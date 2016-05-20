/****************************************************************************
 * (c) Copyright 2015-2016 Hewlett Packard Enterprise Development LP
 *
 *    Licensed under the Apache License, Version 2.0 (the "License"); you may
 *    not use this file except in compliance with the License. You may obtain
 *    a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *    License for the specific language governing permissions and limitations
 *    under the License.
 *
 ***************************************************************************/

#ifndef _QOS_INTF_H_
#define _QOS_INTF_H_

#define QOS_TRUST_KEY "qos_trust"
#define QOS_COS_OVERRIDE_KEY "cos_override"
#define QOS_DSCP_OVERRIDE_KEY "dscp_override"

#define QOS_MAX_QUEUES 8
#define QOS_QUEUE_STATS 3

#define QOS_MAX_BUFFER_SIZE 64
#define QOS_STATUS_QUEUE_PROFILE_KEY "queue_profile"
#define QOS_STATUS_SCHEDULE_PROFILE_KEY "schedule_profile"

void qos_trust_port_show_running_config(const struct ovsrec_port *port_row,
        bool *header_printed, const char *header);

void qos_cos_port_show_running_config(const struct ovsrec_port *port_row,
        bool *header_printed, const char *header);

void qos_apply_port_show_running_config(const struct ovsrec_port *port_row,
        bool *header_printed, const char *header);

void qos_dscp_port_show_running_config(const struct ovsrec_port *port_row,
        bool *header_printed, const char *header);

void qos_trust_port_show(const struct ovsrec_port *port_row,
        const char *interface_name);

void qos_cos_port_show(const struct ovsrec_port *port_row,
        const char *interface_name);

void qos_apply_port_show(const struct ovsrec_port *port_row,
        const char *interface_name);

void qos_dscp_port_show(const struct ovsrec_port *port_row,
        const char *interface_name);

#endif /* _QOS_INTF_H_ */
