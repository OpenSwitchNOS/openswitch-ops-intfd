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
 * @defgroup ops-intfd OpenSwitch Interface Daemon
 *
 * @brief OpenSwitch interface daemon (ops-intfd)
 *
 * The interface daemon manages the Physical Interfaces.
 * Based on the user interface configuration and other parameters which effects
 * the interfaces (like pluggable modules, QSFP splittable configuration, etc.),
 * it derives the opearation state of the Interfaces.
 *
 * @{
 *
 * @file
 * Header for ops-intfd daemon
 *
 * @defgroup intfd_public Public Interface
 * The ops-intfd daemon manages the physical interfaces on the OpenSwitch platform.
 *
 * @{
 *
 * Public APIs
 *
 * Command line options:
 *
 *     ops-intfd: OpenSwitch Interface daemon
 *     usage: ops-intfd [OPTIONS] [DATABASE]
 *     where DATABASE is a socket on which ovsdb-server is listening
 *           (default: "unix:/var/run/openvswitch/db.sock").
 *
 *      Daemon options:
 *        --detach                run in background as daemon
 *        --no-chdir              do not chdir to '/'
 *        --pidfile[=FILE]        create pidfile (default: /var/run/openvswitch/ops-intfd.pid)
 *        --overwrite-pidfile     with --pidfile, start even if already running
 *
 *      Logging options:
 *        -vSPEC, --verbose=SPEC   set logging levels
 *        -v, --verbose            set maximum verbosity level
 *        --log-file[=FILE]        enable logging to specified FILE
 *                                 (default: /var/log/openvswitch/ops-intfd.log)
 *        --syslog-target=HOST:PORT  also send syslog msgs to HOST:PORT via UDP
 *
 *      Other options:
 *        --unixctl=SOCKET        override default control socket name
 *        -h, --help              display this help message
 *
 *
 * Available ovs-apptcl command options are:
 *
 *      coverage/show
 *      exit
 *      list-commands
 *      version
 *      ops-intfd/dump              dumps daemons internal data for debugging.
 *      vlog/disable-rate-limit [module]...
 *      vlog/enable-rate-limit  [module]...
 *      vlog/list
 *      vlog/reopen
 *      vlog/set                {spec | PATTERN:destination:pattern}
 *
 * OVSDB elements usage
 *
 *  The following table rows are READ by ops-intfd:
 *
 *      Interface:name
 *      Interface:pm_info
 *      Interface:user_config
 *      System:cur_cfg
 *
 *  The following columns are WRITTEN by ops-intfd:
 *
 *      Interface:error
 *      Interface:hw_intf_config
 *
 * Linux Files:
 *
 *  The following files are written by ops-intfd:
 *
 *      /var/run/openvswitch/ops-intfd.pid: Process ID for the ops-intfd
 *      /var/run/openvswitch/ops-intfd.<pid>.ctl: Control file for ovs-appctl
 *
 ***************************************************************************/
/** @} end of group intfd_public */

#ifndef __INTFD_H__
#define __INTFD_H__

#include <openvswitch/vlog.h>
#include <openvswitch/compiler.h>

/* Currently only QSFP+ and QSFP28 interfaces can be split
 * up into subintfs, and it's always groups of 4. */
#define MAX_SPLIT_COUNT                4

#define STR_EQ(s1, s2)      ((strlen((s1)) == strlen((s2))) && (!strncmp((s1), (s2), strlen((s2)))))

/*********************************************************************
 * /intf._/info [capabilities] flags
 ********************************************************************/
#define SPEED_1G                    1000
#define SPEED_10G                   10000
#define SPEED_25G                   25000
#define SPEED_40G                   40000
#define SPEED_50G                   50000
#define SPEED_100G                  100000

#define PLUGGABLE_FLAG              (uint64_t)0x00000001
#define ENET_1G_CAPABLE_FLAG        (uint64_t)0x00000002
#define ENET_10G_CAPABLE_FLAG       (uint64_t)0x00000004
#define ENET_25G_CAPABLE_FLAG       (uint64_t)0x00000008
#define ENET_40G_CAPABLE_FLAG       (uint64_t)0x00000010
#define ENET_50G_CAPABLE_FLAG       (uint64_t)0x00000020
#define ENET_100G_CAPABLE_FLAG      (uint64_t)0x00000040
#define INTF_SPLIT_4_CAPABLE_FLAG   (uint64_t)0x00000080

/* Following MACROs check for intf's capabilities. */
#define INTF_IS_PLUGGABLE(p)         (0 != ((p)->pm_info.capability_flags & PLUGGABLE_FLAG))
#define INTF_IS_ENET_1G_CAPABLE(p)   (0 != ((p)->pm_info.capability_flags & ENET_1G_CAPABLE_FLAG))
#define INTF_IS_ENET_10G_CAPABLE(p)  (0 != ((p)->pm_info.capability_flags & ENET_10G_CAPABLE_FLAG))
#define INTF_IS_ENET_25G_CAPABLE(p)  (0 != ((p)->pm_info.capability_flags & ENET_25G_CAPABLE_FLAG))
#define INTF_IS_ENET_40G_CAPABLE(p)  (0 != ((p)->pm_info.capability_flags & ENET_40G_CAPABLE_FLAG))
#define INTF_IS_ENET_50G_CAPABLE(p)  (0 != ((p)->pm_info.capability_flags & ENET_50G_CAPABLE_FLAG))
#define INTF_IS_ENET_100G_CAPABLE(p) (0 != ((p)->pm_info.capability_flags & ENET_100G_CAPABLE_FLAG))
#define INTF_IS_SPLIT_4_CAPABLE(p)   (0 != ((p)->pm_info.capability_flags & INTF_SPLIT_4_CAPABLE_FLAG))

/*********************************************************************
 * op_connector flags to simplify processing
 *********************************************************************
 * NOTE: Use bit 4  (0x0010) to indicate the connector is supported.
 *       Use bit 5  (0x0020) to indicate general SFP module type.
 *       Use bit 6  (0x0040) to indicate SFP+ 10G speed.
 *       Use bit 7  (0x0080) to indicate SFP28 25G speed.
 *       Use bit 7  (0x0100) to indicate general QSFP module type.
 *       Use bit 8  (0x0200) to indicate QSFP+ 40G speed.
 *       Use bit 11 (0x0400) to indicate QSFP28 100G speed.
 ********************************************************************/

/* First, define the OP_CONNECTOR flags */
#define PM_UNSUPPORTED_FLAG         (uint64_t)0x0000
#define PM_SUPPORTED_FLAG           (uint64_t)0x0010
#define PM_SFP_TYPE_FLAG            (uint64_t)0x0020
#define PM_SFP_PLUS_10G_FLAG        (uint64_t)0x0040
#define PM_SFP28_25G_FLAG           (uint64_t)0x0080
#define PM_QSFP_TYPE_FLAG           (uint64_t)0x0100
#define PM_QSFP_PLUS_40G_FLAG       (uint64_t)0x0200
#define PM_QSFP28_100G_FLAG         (uint64_t)0x0400

#define PM_SFP_FLAGS                (PM_SUPPORTED_FLAG | PM_SFP_TYPE_FLAG)

#define PM_SFP_PLUS_FLAGS           (PM_SUPPORTED_FLAG | PM_SFP_TYPE_FLAG | \
                                     PM_SFP_PLUS_10G_FLAG)

#define PM_SFP28_25G_FLAGS          (PM_SUPPORTED_FLAG | PM_SFP_TYPE_FLAG | \
                                     PM_SFP28_25G_FLAG)

#define PM_QSFP_PLUS_40G_FLAGS      (PM_SUPPORTED_FLAG | PM_QSFP_TYPE_FLAG | \
                                     PM_QSFP_PLUS_40G_FLAG)

#define PM_QSFP28_100G_FLAGS        (PM_SUPPORTED_FLAG | PM_QSFP_TYPE_FLAG | \
                                     PM_QSFP28_100G_FLAG)

#define CONNECTOR_SUPPORTED(p)      ((p)->pm_info.op_connector_flags & PM_SUPPORTED_FLAG)
#define CONNECTOR_IS_SFP_FAMILY(p)  ((p)->pm_info.op_connector_flags & PM_SFP_TYPE_FLAG)
#define CONNECTOR_IS_QSFP_FAMILY(p) ((p)->pm_info.op_connector_flags & PM_QSFP_TYPE_FLAG)

#define CONNECTOR_IS_SFP(p)         (CONNECTOR_IS_SFP_FAMILY(p) && \
                                     !((p)->pm_info.op_connector_flags & \
                                     (PM_SFP_PLUS_10G_FLAG | PM_SFP28_25G_FLAG)))

#define CONNECTOR_IS_SFP_PLUS_10G(p) (CONNECTOR_IS_SFP_FAMILY(p) && \
                                      ((p)->pm_info.op_connector_flags & PM_SFP_PLUS_10G_FLAG))

#define CONNECTOR_IS_SFP28_25G(p)    (CONNECTOR_IS_SFP_FAMILY(p) && \
                                      ((p)->pm_info.op_connector_flags & PM_SFP28_25G_FLAG))

#define CONNECTOR_IS_QSFP_PLUS_40G(p) (CONNECTOR_IS_QSFP_FAMILY(p) && \
                                       ((p)->pm_info.op_connector_flags & PM_QSFP_PLUS_40G_FLAG))

#define CONNECTOR_IS_QSFP28_100G(p)    (CONNECTOR_IS_QSFP_FAMILY(p) && \
                                        ((p)->pm_info.op_connector_flags & PM_QSFP28_100G_FLAG))

#define CONNECTOR_IS_SFP_RJ45(p) (INTERFACE_PM_INFO_CONNECTOR_SFP_RJ45 == (p)->pm_info.op_connector)
#define CONNECTOR_IS_DAC(p)      (INTERFACE_PM_INFO_CONNECTOR_SFP_DAC == (p)->pm_info.op_connector)
#define CONNECTOR_IS_1G(p)       (CONNECTOR_IS_SFP(p))
#define CONNECTOR_IS_10G(p)      (CONNECTOR_IS_SFP_PLUS_10G(p))
#define CONNECTOR_IS_ABSENT(p)   (INTERFACE_PM_INFO_CONNECTOR_ABSENT == (p)->pm_info.op_connector)

#define CABLE_TECH_IS_ACTIVE(p)  (INTERFACE_PM_INFO_CABLE_TECHNOLOGY_ACTIVE == (p)->pm_info.cable_tech)
#define CABLE_TECH_IS_PASSIVE(p) (INTERFACE_PM_INFO_CABLE_TECHNOLOGY_PASSIVE == (p)->pm_info.cable_tech)

#define INTFD_MIN_ALLOWED_USER_SPECIFIED_MTU     576
#define INTFD_DEFAULT_MTU                       1500
#define INTFD_MAX_SPEEDS_ALLOWED                  10
#define INTFD_AUTONEG_STATE_INVALID               -1
#define INTFD_AUTONEG_STATE_DISABLED               0
#define INTFD_AUTONEG_STATE_ENABLED                1
#define INTFD_AUTONEG_CAPABILITY_UNSUPPORTED      10
#define INTFD_AUTONEG_CAPABILITY_OPTIONAL         11
#define INTFD_AUTONEG_CAPABILITY_REQUIRED         12

/* A protocol object part of some forwarding layer object */
struct intfd_arbiter_proto_class {
    /* The id associated with the protocol */
    enum ovsrec_interface_forwarding_state_proto_e id;

    /* The forwarding layer to which this protocol belongs to */
    struct intfd_arbiter_layer_class *layer;

    /* Function that runs and determines if the forwarding state
     * needs to change based on the current protocols state */
    bool (*run) (struct intfd_arbiter_proto_class *proto,
                 const struct ovsrec_interface *ifrow);

    /* Function that returns the protocols view of the forwarding
     * state of the interface. */
    bool (*get_state) (const struct ovsrec_interface *ifrow);

    /* Next protocol in the forwarding layer */
    struct intfd_arbiter_proto_class *next;
};

/* A forwarding layer object */
struct intfd_arbiter_layer_class {
    /* The id associated with the forwarding layer */
    enum ovsrec_interface_forwarding_state_layer_e id;

    /* The protocol that is currently determining the state of the forwarding layer */
    enum ovsrec_interface_forwarding_state_proto_e owner;

    /* A list of protocols operating at this layer.
     * The order of the list determines precedence among protocols.
     * The protocol at the lower index trumps the one at a higher index. */
    struct intfd_arbiter_proto_class *protos;

    /* Boolean variable that tells if the forwarding state of the give layer is blocked */
    bool blocked;

    /* Function that determines if the forwarding state of the layer has to change */
    bool (*run) (struct intfd_arbiter_layer_class *layer,
                 const struct ovsrec_interface *ifrow);

    /* Pointer to the previous forwarding layer in the hierarchy */
    struct intfd_arbiter_layer_class *prev;

    /* Pointer to the next forwarding layer in the hierarchy */
    struct intfd_arbiter_layer_class *next;
};

/* An interface arbiter object */
struct intfd_arbiter_class {
    /* A list of forwarding layers applicable for this object */
    struct intfd_arbiter_layer_class *layers;
};

extern void intfd_ovsdb_init(const char *db_path);
extern void intfd_ovsdb_exit(void);
extern void intfd_run(void);
extern void intfd_wait(void);
extern void intfd_debug_dump(struct ds *ds, int argc, const char *argv[]);
extern void intfd_arbiter_init(void);
extern void intfd_arbiter_interface_run(const struct ovsrec_interface *ifrow,
        struct smap *forwarding_state);
#endif /* __INTFD_H__ */
/** @} end of group ops-intfd */
