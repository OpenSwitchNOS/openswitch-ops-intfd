/*
 * (c) Copyright 2016 Hewlett Packard Enterprise Development LP
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

#include <smap.h>
#include <openvswitch/vlog.h>

#include <openswitch-idl.h>
#include <vswitch-idl.h>

#include "intfd.h"

VLOG_DEFINE_THIS_MODULE(intfd_arbiter);

struct intfd_arbiter_class intfd_arbiter;

/*!
 * @brief      Function to return the precedence associated with a give protocol.
 *
 * @param[in]  name      The name of the protocol
 *
 * @return     The precedence (enumeration value) associated with the
 *             protocol passed.
 */
enum ovsrec_interface_proto_precedence_e intfd_arbiter_get_proto_precendence(
        const char *name)
{
    if (STR_EQ(name, INTERFACE_FORWARDING_STATE_PROTOCOL_UDLD)) {
        return INTERFACE_PROTO_UDLD;
    } else if (STR_EQ(name, INTERFACE_FORWARDING_STATE_PROTOCOL_DOT1X)) {
        return INTERFACE_PROTO_DOT1X;
    } else if (STR_EQ(name, INTERFACE_FORWARDING_STATE_PROTOCOL_LACP)) {
        return INTERFACE_PROTO_LACP;
    } else {
        return INTERFACE_PROTO_NONE;
    }
}

/*!
 * @brief      A utility function to attach a new protocol to the existing list
 *             of protocols of a forwarding layer.
 *
 * @param[in,out]  head     The head of the linked list of protocols.
 * @param[in]      proto    The new protocol to be attached to the list.
 *
 * @return     Nothing
 */
void intfd_arbiter_attach_proto(struct intfd_arbiter_proto_class **head,
                                struct intfd_arbiter_proto_class *proto)
{
    struct intfd_arbiter_proto_class *node = *head;

    /* If the head is null, list is empty. Return the current node as head */
    if (node == NULL) {
        *head = proto;
    } else {
        /* Walk the list from head till we reach the last node */
        while (node->next != NULL) {
            node = node->next;
        }
        /* Attach the new node to the list */
        node->next = proto;
    }
}

/*!
 * @brief      A utility function to attach a new forwarding layer to the
 *             existing list forwarding layers for an interface.
 *
 * @param[in,out]  head     The head of the linked list of forwarding layers.
 * @param[in]      layer    The new layer to be attached to the list.
 *
 * @return     Nothing
 */
void intfd_arbiter_attach_layer(struct intfd_arbiter_layer_class **head,
                                struct intfd_arbiter_layer_class *layer)
{
    struct intfd_arbiter_layer_class *node = *head;

    /* If the head is null, list is empty. Return the current node as head. */
    if (node == NULL) {
        *head = layer;
    } else {
        /* Walk the list from head till we reach the last node */
        while (node->next != NULL) {
            node = node->next;
        }
        /* Attach the new node to the list and point it to the previous node. */
        node->next = layer;
        layer->prev = node;
    }
}

/*!
 * @brief      Callback function to run the arbiter algorithm for a given
 *             protocol operating at a given layer of a given interface.
 *
 * @param[in]       proto     The pointer to the protocol data structure.
 * @param[in]       ifrow     The interface for which the arbiter is running.
 * @param[in,out]   forwarding_state The forwarding state column of OVSDB.
 *
 * @return     true     If the current run deemed the forwarding state of the
 *                      interface layer to be blocked.
 *             false    If the current run deemed the forwarding state of the
 *                      interface layer to not be blocked.
 */
bool intfd_arbiter_proto_run(struct intfd_arbiter_proto_class *proto,
                             const struct ovsrec_interface *ifrow,
                             struct smap *forwarding_state)
{
    bool block;
    const char *owner;
    block = (proto->state) ? proto->state(ifrow, forwarding_state) : false;

    owner = smap_get(forwarding_state, proto->layer->owner);

    if (block) {
        /* Check if the current forwarding state of this layer is already blocked. */
        if (STR_EQ(smap_get(forwarding_state, proto->layer->name),
                   INTERFACE_FORWARDING_STATE_BLOCKED)) {
            /* Check if the current asserting protocol is of lower precedence than the current protocol.
             * If yes, change the owner to current protocol.  */
            if (!owner || (intfd_arbiter_get_proto_precendence(proto->name)
                    < intfd_arbiter_get_proto_precendence(owner))) {
                VLOG_INFO(
                        "Changing owner of %s to %s for interface %s",
                        proto->layer->name, proto->name, ifrow->name);
                smap_replace(forwarding_state, proto->layer->owner,
                             proto->name);
            }
        } else {
            VLOG_INFO(
                    "Changing status of %s to blocked with owner as %s for interface %s",
                    proto->layer->name, proto->name, ifrow->name);
            /* Set the forwarding state for this layer to block and set the owner as current protocol. */
            smap_replace(forwarding_state, proto->layer->name,
                         INTERFACE_FORWARDING_STATE_BLOCKED);
            smap_replace(forwarding_state, proto->layer->owner, proto->name);
        }
    } else {
        /* Check if the current forwarding state of this layer is already blocked. */
        if (STR_EQ(smap_get(forwarding_state, proto->layer->name),
                   INTERFACE_FORWARDING_STATE_BLOCKED)) {
            /* Check if current protocol is the current owner.
             * If yes, clear the owner and move the state to forwarding. */
            if (owner && STR_EQ(owner, proto->name)) {
                VLOG_INFO(
                        "Changing status of %s to forwarding with owner %s cleared for interface %s",
                        proto->layer->name, proto->name, ifrow->name);
                smap_remove(forwarding_state, proto->layer->owner);
                smap_replace(forwarding_state, proto->layer->name,
                             INTERFACE_FORWARDING_STATE_FORWARDING);
            }
        }
    }

    return block;
}

/*!
 * @brief      Callback function to run the arbiter algorithm for a given
 *             forwarding layer of a given interface.
 *
 * @param[in]       layer     The pointer to the f/w layer data structure.
 * @param[in]       ifrow     The interface for which the arbiter is running.
 * @param[in,out]   forwarding_state The forwarding state column of OVSDB.
 *
 * @return     true     If the current run deemed the forwarding state of the
 *                      interface layer to be blocked.
 *             false    If the current run deemed the forwarding state of the
 *                      interface layer to not be blocked.
 */
bool intfd_arbiter_layer_run(struct intfd_arbiter_layer_class *layer,
                             const struct ovsrec_interface *ifrow,
                             struct smap *forwarding_state)
{
    struct intfd_arbiter_proto_class *proto;
    bool block;
    const char *admin_state;

    /* If this is the first ever run for this layer, set the state as forwarding */
    if (!smap_get(forwarding_state, layer->name)) {
        VLOG_INFO(
                "Setting status of %s to forwarding as default for interface %s",
                layer->name, ifrow->name);
        smap_add(forwarding_state, layer->name,
                 INTERFACE_FORWARDING_STATE_FORWARDING);
    }

    /* Check if the admin state of the interface is down */
    admin_state = smap_get((const struct smap *) &ifrow->user_config,
    INTERFACE_USER_CONFIG_MAP_ADMIN);
    if (!admin_state || !(STR_EQ(admin_state,
                                 OVSREC_INTERFACE_USER_CONFIG_ADMIN_UP))) {
        /* Set the current layer as blocked and remove the owner. */
        smap_replace(forwarding_state, layer->name,
                     INTERFACE_FORWARDING_STATE_BLOCKED);
        smap_remove(forwarding_state, layer->owner);
        VLOG_INFO("Blocking interface %s because admin state is down",
                  ifrow->name);
        return true;
    }

    /* Check if the forwarding state of the previous layer is blocked. */
    if (layer->prev && STR_EQ(layer->prev->state(layer->prev, forwarding_state),
                              INTERFACE_FORWARDING_STATE_BLOCKED)) {
        /* Set the current layer as blocked and remove the owner. */
        VLOG_INFO(
                "Blocking forwarding layer %s of interface %s because forwarding layer %s is blocked",
                layer->name, ifrow->name, layer->prev->name);
        smap_replace(forwarding_state, layer->name,
                     INTERFACE_FORWARDING_STATE_BLOCKED);
        smap_remove(forwarding_state, layer->owner);
        return true;
    }

    proto = layer->protos;

    /* Walk through all the protocols operating at this layer and determine the new forwarding state */
    while (proto != NULL) {
        if (proto->run) {
            block = proto->run(proto, ifrow, forwarding_state);
            if (block) {
                return true;
            }
        }
        proto = proto->next;
    }

    /* None of the protocols set the layer as blocking.
     * Move the state to forwarding */
    /* TODO: Dump only if there was an actual state change */
    VLOG_INFO("Changing status of %s to forwarding for interface %s",
              layer->name, ifrow->name);
    smap_replace(forwarding_state, layer->name,
                 INTERFACE_FORWARDING_STATE_FORWARDING);
    smap_remove(forwarding_state, layer->owner);

    return false;
}

/*!
 * @brief      Function to run the arbiter algorithm for a given interface.
 *
 * @param[in]       ifrow     The interface for which the arbiter is running.
 * @param[in,out]   forwarding_state The forwarding state column of OVSDB.
 *
 * @return     Nothing
 */
void intfd_arbiter_interface_run(const struct ovsrec_interface *ifrow,
                                 struct smap *forwarding_state)
{
    struct intfd_arbiter_layer_class *last_layer, *layer;
    last_layer = layer = intfd_arbiter.layers;

    /* Walk from the first to last applicable forwarding layers for interface */
    while (layer != NULL) {
        /* Trigger the current layer checks if it has a registered function. */
        if (layer->run) {
            layer->run(layer, ifrow, forwarding_state);
        }

        /* Move to the next forwarding layer in the hierarchy */
        last_layer = layer;
        layer = layer->next;
    }

    /* Set the forwarding state of the interface based on the forwarding state
     * of the last layer.
     * If there isn't one, set the interface state as forwarding. */
    if (last_layer && last_layer->state) {
        smap_replace(forwarding_state, INTERFACE_FORWARDING_STATE,
                     last_layer->state(last_layer, forwarding_state));
    } else {
        smap_replace(forwarding_state, INTERFACE_FORWARDING_STATE,
                     INTERFACE_FORWARDING_STATE_FORWARDING);
    }
}

/*!
 * @brief      Function to determine the forwarding state of an interface
 *             from "LACP" perspective.
 *
 * @param[in]       ifrow     The interface for which the arbiter is running.
 * @param[in,out]   forwarding_state The forwarding state column of OVSDB.
 *
 * @return     true     If lacp deems the interface should be blocked.
 *             false    If lacp deems the interface should be forwarding.
 */
bool intfd_arbiter_lacp_state(const struct ovsrec_interface *ifrow,
                              struct smap *forwarding_state)
{
    /* Get the forwarding state for this protocol */
    if (smap_get(&ifrow->hw_bond_config,
                 INTERFACE_HW_BOND_CONFIG_MAP_RX_ENABLED)
        && smap_get(&ifrow->hw_bond_config,
                    INTERFACE_HW_BOND_CONFIG_MAP_TX_ENABLED)
        && STR_EQ(smap_get(&ifrow->hw_bond_config,
                           INTERFACE_HW_BOND_CONFIG_MAP_RX_ENABLED),
                "false")
        && STR_EQ(smap_get(&ifrow->hw_bond_config,
                           INTERFACE_HW_BOND_CONFIG_MAP_TX_ENABLED),
                "false")) {
        return true;
    } else {
        return false;
    }
}

const char *
intfd_arbiter_layer_state(struct intfd_arbiter_layer_class *layer,
                          struct smap *forwarding_state)
{
    return smap_get(forwarding_state, layer->name);
}

/*!
 * @brief      Function to register the forwarding layer 'health' and all the
 *             applicable protocols at this layer with the arbiter.
 *
 * @param[in,out]  layer_head     The head of the linked list of forwarding layers.
 *
 * @return     Nothing
 */
void intfd_arbiter_layer_health_register(
        struct intfd_arbiter_layer_class **layer_head)
{
    struct intfd_arbiter_layer_class *health;
    struct intfd_arbiter_proto_class *proto_head = NULL;

    /* Define and register the health layer */
    health = calloc(1, sizeof(struct intfd_arbiter_layer_class));
    health->name = INTERFACE_FORWARDING_STATE_INTERFACE_HEALTH;
    health->owner = INTERFACE_FORWARDING_STATE_INTERFACE_HEALTH_OWNER;
    health->run = intfd_arbiter_layer_run;
    health->state = intfd_arbiter_layer_state;
    health->next = NULL;
    health->prev = NULL;

    /* Define and register the protocols running at health layer */

    health->protos = proto_head;

    /* Attach the health layer to the list */
    intfd_arbiter_attach_layer(layer_head, health);

    return;
}

/*!
 * @brief      Function to register the forwarding layer 'security' and all the
 *             applicable protocols at this layer with the arbiter.
 *
 * @param[in,out]  layer_head     The head of the linked list of forwarding layers.
 *
 * @return     Nothing
 */
void intfd_arbiter_layer_security_register(
        struct intfd_arbiter_layer_class **layer_head)
{
    struct intfd_arbiter_layer_class *security;
    struct intfd_arbiter_proto_class *proto_head = NULL;

    /* Define and register the security layer */
    security = calloc(1, sizeof(struct intfd_arbiter_layer_class));
    security->name = INTERFACE_FORWARDING_STATE_INTERFACE_SECURITY;
    security->owner = INTERFACE_FORWARDING_STATE_INTERFACE_SECURITY_OWNER;
    security->run = intfd_arbiter_layer_run;
    security->state = intfd_arbiter_layer_state;
    security->next = NULL;
    security->prev = NULL;

    /* Define and register the protocols running at security layer */

    security->protos = proto_head;

    /* Attach the health layer to the list */
    intfd_arbiter_attach_layer(layer_head, security);

    return;
}

/*!
 * @brief      Function to register the forwarding layer 'aggregation' and all the
 *             applicable protocols at this layer with the arbiter.
 *
 * @param[in,out]  layer_head     The head of the linked list of forwarding layers.
 *
 * @return     Nothing
 */
void intfd_arbiter_layer_aggregation_register(
        struct intfd_arbiter_layer_class **layer_head)
{
    struct intfd_arbiter_layer_class *aggregation;
    struct intfd_arbiter_proto_class *proto, *proto_head = NULL;

    /* Define and register the aggregation layer */
    aggregation = calloc(1, sizeof(struct intfd_arbiter_layer_class));
    aggregation->name = INTERFACE_FORWARDING_STATE_INTERFACE_AGGREGATION;
    aggregation->owner = INTERFACE_FORWARDING_STATE_INTERFACE_AGGREGATION_OWNER;
    aggregation->run = intfd_arbiter_layer_run;
    aggregation->state = intfd_arbiter_layer_state;
    aggregation->next = NULL;
    aggregation->prev = NULL;

    /* Define and register the protocols running at aggregation layer */

    /* Register LACP */
    proto = calloc(1, sizeof(struct intfd_arbiter_proto_class));
    proto->name = INTERFACE_FORWARDING_STATE_PROTOCOL_LACP;
    proto->run = intfd_arbiter_proto_run;
    proto->state = intfd_arbiter_lacp_state;
    proto->layer = aggregation;
    proto->next = NULL;
    intfd_arbiter_attach_proto(&proto_head, proto);

    aggregation->protos = proto_head;

    /* Attach the health layer to the list */
    intfd_arbiter_attach_layer(layer_head, aggregation);

    return;
}

/*!
 * @brief      Function to initialize the interface arbiter.
 *
 * @return     Nothing
 */
void intfd_arbiter_init(void)
{
    memset(&intfd_arbiter, 0, sizeof(struct intfd_arbiter_class));

    /* Initialize and attach the interface 'health' layer */
    intfd_arbiter_layer_health_register(&intfd_arbiter.layers);
    /* Initialize and attach the interface 'security' layer */
    intfd_arbiter_layer_security_register(&intfd_arbiter.layers);
    /* Initialize and attach the interface 'aggregation' layer */
    intfd_arbiter_layer_aggregation_register(&intfd_arbiter.layers);

    return;
}
