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

#include "vtysh/command.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include "vswitch-idl.h"
#include "ovsdb-idl.h"
#include "qos_intf.h"
#include "smap.h"
#include "memory.h"
#include "openvswitch/vlog.h"
#include "openswitch-idl.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_ovsdb_config.h"

extern struct ovsdb_idl *idl;

/**
 * Returns the lag for which the given port_name is a member, or
 * NULL if the port_name is not a member of a lag.
 */
static const struct ovsrec_port * get_owning_lag(const char *port_name) {
    const struct ovsrec_port *port_row;
    OVSREC_PORT_FOR_EACH(port_row, idl) {
        int i;
        for (i = 0; i < port_row->n_interfaces; i++) {
            if ((strcmp(port_row->interfaces[i]->name, port_name) == 0)
                    && (strcmp(port_row->name, port_name) != 0)) {
                return port_row;
            }
        }
    }

    return NULL;
}

static bool is_member_of_lag(const char *port_name) {
    bool is_member = get_owning_lag(port_name) != NULL;
    return is_member;
}

void qos_trust_port_show_running_config(const struct ovsrec_port *port_row,
        bool *header_printed, const char *header) {
    if (port_row == NULL) {
        return;
    }

    if (is_member_of_lag(port_row->name)) {
        return;
    }

    const char *qos_trust_name = smap_get(&port_row->qos_config, QOS_TRUST_KEY);
    if (qos_trust_name == NULL) {
        return;
    }

    if (!*header_printed) {
        *header_printed = true;
        vty_out(vty, "%s %s%s", header, port_row->name, VTY_NEWLINE);
    }
    vty_out(vty, "    qos trust %s%s", qos_trust_name, VTY_NEWLINE);
}

void qos_cos_port_show_running_config(const struct ovsrec_port *port_row,
        bool *header_printed, const char *header) {
    if (port_row == NULL) {
        return;
    }

    if (is_member_of_lag(port_row->name)) {
        return;
    }

    const char *cos_map_index = smap_get(&port_row->qos_config,
            QOS_COS_OVERRIDE_KEY);
    if (cos_map_index == NULL) {
        return;
    }

    if (!*header_printed) {
        *header_printed = true;
        vty_out(vty, "%s %s%s", header, port_row->name, VTY_NEWLINE);
    }
    vty_out(vty, "    qos cos %s%s", cos_map_index, VTY_NEWLINE);
}

void qos_apply_port_show_running_config(const struct ovsrec_port *port_row,
        bool *header_printed, const char *header) {
    if (port_row == NULL) {
        return;
    }

    if (is_member_of_lag(port_row->name)) {
        return;
    }

    /* Show the schedule profile. */
    if (port_row->qos != NULL) {
        const char *schedule_profile_name = port_row->qos->name;
        if (schedule_profile_name != NULL) {
            if (!*header_printed) {
                *header_printed = true;
                vty_out(vty, "%s %s%s", header, port_row->name, VTY_NEWLINE);
            }
            vty_out(vty, "    apply qos schedule-profile %s%s",
                    schedule_profile_name, VTY_NEWLINE);
        }
    }
}

void qos_dscp_port_show_running_config(const struct ovsrec_port *port_row,
        bool *header_printed, const char *header) {
    if (port_row == NULL) {
        return;
    }

    if (is_member_of_lag(port_row->name)) {
        return;
    }

    const char *dscp_map_index = smap_get(&port_row->qos_config,
            QOS_DSCP_OVERRIDE_KEY);
    if (dscp_map_index == NULL) {
        return;
    }

    if (!*header_printed) {
        *header_printed = true;
        vty_out(vty, "%s %s%s", header, port_row->name, VTY_NEWLINE);
    }
    vty_out(vty, "    qos dscp %s%s", dscp_map_index, VTY_NEWLINE);
}

void qos_trust_port_show(const struct ovsrec_port *port_row,
        const char *interface_name) {
    const struct ovsrec_system *system_row = ovsrec_system_first(idl);
    const char *qos_trust_name = smap_get(
            &system_row->qos_config, QOS_TRUST_KEY);

    const struct ovsrec_port *port_to_show = port_row;

    const struct ovsrec_port *owning_lag = get_owning_lag(interface_name);
    if (owning_lag != NULL) {
        port_to_show = owning_lag;
    }

    if (port_to_show != NULL) {
        const char *map_value = smap_get(&port_to_show->qos_config,
                QOS_TRUST_KEY);
        if (map_value != NULL) {
            qos_trust_name = map_value;
        }
    }

    vty_out(vty, " qos trust %s%s", qos_trust_name, VTY_NEWLINE);
}

void qos_cos_port_show(const struct ovsrec_port *port_row,
        const char *interface_name) {
    const struct ovsrec_port *port_to_show = port_row;

    const struct ovsrec_port *owning_lag = get_owning_lag(interface_name);
    if (owning_lag != NULL) {
        port_to_show = owning_lag;
    }

    if (port_to_show == NULL) {
        return;
    }

    const char *cos_map_index = smap_get(&port_to_show->qos_config,
            QOS_COS_OVERRIDE_KEY);
    if (cos_map_index == NULL) {
        return;
    }

    vty_out(vty, " qos cos override %s%s", cos_map_index, VTY_NEWLINE);
}

void qos_apply_port_show(const struct ovsrec_port *port_row,
        const char *interface_name) {
    const struct ovsrec_system *system_row = ovsrec_system_first(idl);
    const char *queue_profile_name = system_row->q_profile->name;
    const char *schedule_profile_name = system_row->qos->name;
    const char *queue_profile_status = NULL;
    const char *schedule_profile_status = NULL;

    const struct ovsrec_port *port_to_show = port_row;

    const struct ovsrec_port *owning_lag = get_owning_lag(interface_name);
    if (owning_lag != NULL) {
        port_to_show = owning_lag;
    }

    if (port_to_show != NULL) {
        if (port_to_show->q_profile != NULL) {
            queue_profile_name = port_to_show->q_profile->name;
        }

        if (port_to_show->qos != NULL) {
            schedule_profile_name = port_to_show->qos->name;
        }

        queue_profile_status = smap_get(&port_to_show->qos_status,
                QOS_STATUS_QUEUE_PROFILE_KEY);
        schedule_profile_status = smap_get(&port_to_show->qos_status,
                QOS_STATUS_SCHEDULE_PROFILE_KEY);
    }

    /* Show queue profile. */
    if (queue_profile_status != NULL && strncmp(queue_profile_name,
            queue_profile_status, QOS_MAX_BUFFER_SIZE) != 0) {
        /* Print the status, if it differs from the config. */
        vty_out(vty, " qos queue-profile %s, status is %s%s",
                queue_profile_name,
                queue_profile_status,
                VTY_NEWLINE);
    } else {
        vty_out(vty, " qos queue-profile %s%s",
                queue_profile_name,
                VTY_NEWLINE);
    }

    /* Show schedule profile. */
    if (schedule_profile_status != NULL && strncmp(schedule_profile_name,
            schedule_profile_status, QOS_MAX_BUFFER_SIZE) != 0) {
        /* Print the status, if it differs from the config. */
        vty_out(vty, " qos schedule-profile %s, status is %s%s",
                schedule_profile_name,
                schedule_profile_status,
                VTY_NEWLINE);
    } else {
        vty_out(vty, " qos schedule-profile %s%s",
                schedule_profile_name,
                VTY_NEWLINE);
    }
}

void qos_dscp_port_show(const struct ovsrec_port *port_row,
        const char *interface_name) {
    const struct ovsrec_port *port_to_show = port_row;

    const struct ovsrec_port *owning_lag = get_owning_lag(interface_name);
    if (owning_lag != NULL) {
        port_to_show = owning_lag;
    }

    if (port_to_show == NULL) {
        return;
    }

    const char *dscp_map_index = smap_get(&port_to_show->qos_config,
            QOS_DSCP_OVERRIDE_KEY);
    if (dscp_map_index == NULL) {
        return;
    }

    vty_out(vty, " qos dscp override %s%s", dscp_map_index, VTY_NEWLINE);
}
