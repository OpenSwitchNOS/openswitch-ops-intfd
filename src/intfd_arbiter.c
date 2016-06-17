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
 * @brief      A utility function to get the value associated with
 *             a given interface layer protocol.
 *
 * @param[in]  proto    The enumeration value of the protocol.
 *
 * @return     The name associated with the protocol.
 */
const char *
intfd_arbiter_get_proto_name(enum ovsrec_interface_forwarding_state_proto_e proto)
{
    switch (proto) {
        case INTERFACE_FORWARDING_STATE_PROTO_LACP:
            return INTERFACE_FORWARDING_STATE_PROTOCOL_LACP;
        default:
            return NULL;
    }
}

/*!
 * @brief      A utility function to get OVSDB key name for
 *             the forwarding state for a given layer.
 *
 * @param[in]  layer    The enumeration value of the layer.
 *
 * @return     The OVSDB key name associated with the layer.
 */
const char *
intfd_arbiter_get_layer_key(enum ovsrec_interface_forwarding_state_layer_e layer)
{
    switch (layer) {
        case INTERFACE_FORWARDING_STATE_LAYER_AGGREGATION:
            return INTERFACE_FORWARDING_STATE_MAP_INTERFACE_AGGREGATION_FORWARDING;
        default:
            return NULL;
    }
}

/*!
 * @brief      A utility function to get OVSDB key name for
 *             the asserting protocol for a given layer.
 *
 * @param[in]  layer    The enumeration value of the layer.
 *
 * @return     The OVSDB key name associated with the asserting
 *             protocol.
 */
const char *
intfd_arbiter_get_layer_owner_key(enum ovsrec_interface_forwarding_state_layer_e layer)
{
    switch (layer) {
        case INTERFACE_FORWARDING_STATE_LAYER_AGGREGATION:
            return INTERFACE_FORWARDING_STATE_MAP_INTERFACE_AGGREGATION_BLOCKED_REASON;
        default:
            return NULL;
    }
}

/*!
 * @brief      A utility function to translate the boolean
 *             state value to string stored in OVSDB.
 *
 * @param[in]  blocked    Boolean value to denote blocked or not.
 *
 * @return     String associated with the forwarding state.
 */
const char *
intfd_arbiter_get_state_value(bool blocked)
{
    return blocked ?
            INTERFACE_FORWARDING_STATE_FORWARDING_FALSE:
            INTERFACE_FORWARDING_STATE_FORWARDING_TRUE;
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
void
intfd_arbiter_attach_proto(struct intfd_arbiter_proto_class **head,
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
void
intfd_arbiter_attach_layer(struct intfd_arbiter_layer_class **head,
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
        /* Attach the new node to the list and point it to the previous node */
        node->next = layer;
        layer->prev = node;
    }
}

/*!
 * @brief      Callback function to run the arbiter algorithm for a given
 *             protocol operating at a given layer of a given interface.
 *
 * @param[in]  proto     The pointer to the protocol data structure.
 * @param[in]  ifrow     The interface for which the arbiter is running.
 *
 * @return     true     If the current run deemed the forwarding state of the
 *                      interface layer to be blocked.
 *             false    If the current run deemed the forwarding state of the
 *                      interface layer to not be blocked.
 */
bool
intfd_arbiter_proto_run(struct intfd_arbiter_proto_class *proto,
                        const struct ovsrec_interface *ifrow)
{
    bool block;

    /* Get the view of the forwarding state for the interface for
     * the current protocol. */
    block = (proto->get_state) ? proto->get_state(ifrow) : false;

    if (block) {
        /* Check if the current forwarding state of this layer is already
         * blocked. */
        if (proto->layer->blocked) {
            /* Check if the current asserting protocol is of lower precedence
             * than the current protocol.
             * If yes, change the owner to current protocol. */
            if (proto->id < proto->layer->owner) {
                VLOG_DBG(
                        "Changing owner of %d to %d for interface %s",
                        proto->layer->id, proto->id, ifrow->name);
                proto->layer->owner = proto->id;
            }
        } else {
            VLOG_DBG(
                    "Changing status of %d to blocked with owner as %d "
                    "for interface %s",
                    proto->layer->id, proto->id, ifrow->name);
            /* Set the forwarding state for this layer to block and set the
             * owner as current protocol. */
            proto->layer->blocked = true;
            proto->layer->owner = proto->id;
        }
    } else {
        /* Check if the current forwarding state of this layer is
         * already blocked. */
        if (proto->layer->blocked) {
            /* Check if current protocol is the current owner.
             * If yes, clear the owner and move the state to forwarding. */
            if (proto->id == proto->layer->owner) {
                VLOG_DBG(
                        "Changing status of %d to forwarding with owner %d "
                        "cleared for interface %s",
                        proto->layer->id, proto->id, ifrow->name);
                proto->layer->owner = INTERFACE_FORWARDING_STATE_PROTO_NONE;
                proto->layer->blocked = false;
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
 *
 * @return     true     If the current run deemed the forwarding state of the
 *                      interface layer to be blocked.
 *             false    If the current run deemed the forwarding state of the
 *                      interface layer to not be blocked.
 */
bool
intfd_arbiter_layer_run(struct intfd_arbiter_layer_class *layer,
                        const struct ovsrec_interface *ifrow)
{
    struct intfd_arbiter_proto_class *proto;
    bool block;
    const char *oper_state;
#ifdef NOT_YET
    const char *hw_status;
#endif /* NOT_YET */

    /* Check if the forwarding state of the previous layer is blocked. */
    if (layer->prev && layer->prev->blocked) {
        /* Set the current layer as blocked and remove the owner. */
        VLOG_DBG(
                "Blocking %d for interface %s because forwarding layer "
                "%d is blocked",
                layer->id, ifrow->name, layer->prev->id);
        layer->blocked = true;
        layer->owner = INTERFACE_FORWARDING_STATE_PROTO_NONE;
        return true;
    }

    /* Get the operator state of the interface */
    oper_state = smap_get(&ifrow->hw_intf_config,
                          INTERFACE_HW_INTF_CONFIG_MAP_ENABLE);
    /* Block the interface if:
     * a. The operator status of the interface is down.
     * b. The hardware ready state of the interface is down.
     */
    if (!oper_state ||
        !(STR_EQ(oper_state,
                 INTERFACE_HW_INTF_CONFIG_MAP_ENABLE_TRUE))) {
        /* Set the current layer as blocked and remove the owner. */
        if (!layer->blocked) {
            VLOG_DBG("Blocking %d for interface %s because the operator "
                    "state is down", layer->id, ifrow->name);
        }

        layer->blocked = true;
        layer->owner = INTERFACE_FORWARDING_STATE_PROTO_NONE;
        return true;
    }

#ifdef NOT_YET
    /* TODO: Enable this block once ACLs are ready */
    /* Get the hw_status of the interface */
    hw_status = smap_get(&ifrow->hw_status,
                         "ready");
    if (!hw_status ||
        !(STR_EQ(hw_status,
                 "true"))) {
        /* Set the current layer as blocked and remove the owner. */
        if (!layer->blocked) {
            VLOG_DBG("Blocking %d for interface %s because the h/w status "
                    "is blocked", layer->id, ifrow->name);
        }

        layer->blocked = true;
        layer->owner = INTERFACE_FORWARDING_STATE_PROTO_NONE;
        return true;
    }
#endif /* NOT_YET */

    proto = layer->protos;

    /* Walk through all the protocols operating at this layer and determine the
     * new forwarding state */
    while (proto != NULL) {
        if (proto->run) {
            block = proto->run(proto, ifrow);
            if (block) {
                return true;
            }
        }
        proto = proto->next;
    }

    /* None of the protocols set the layer as blocking.
     * Move the state to forwarding */
    layer->blocked = false;
    layer->owner = INTERFACE_FORWARDING_STATE_PROTO_NONE;

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
void
intfd_arbiter_interface_run(const struct ovsrec_interface *ifrow,
                            struct smap *forwarding_state)
{
    struct intfd_arbiter_layer_class *last_layer, *layer;
    const char *layer_key, *layer_owner_key, *owner_name, *state_value;

    last_layer = layer = intfd_arbiter.layers;

    /* Walk from the first to last applicable forwarding layers for interface */
    while (layer != NULL) {
        /* Trigger the current layer checks if it has a registered function. */
        if (layer->run) {
            layer->run(layer, ifrow);
        }

        /* Get OVSDB key name for setting the forwarding state of the
         * current layer */
        layer_key = intfd_arbiter_get_layer_key(layer->id);
        /* Get OVSDB key name for setting the owner for this layer dictating
         * the forwarding state. */
        layer_owner_key = intfd_arbiter_get_layer_owner_key(layer->id);
        /* Get name for the current asserting owner for this layer */
        owner_name = intfd_arbiter_get_proto_name(layer->owner);
        /* Get the value associated with the forwarding state of the current
         * layer */
        state_value = intfd_arbiter_get_state_value(layer->blocked);

        /* Check if the current layer has an owner */
        if (layer->owner != INTERFACE_FORWARDING_STATE_PROTO_NONE) {
            /* There is an owner. Set the forwarding state and the owner
             * for this layer based on the information cached in the layer
             * data structure. */
            smap_replace(forwarding_state, layer_key, state_value);
            smap_replace(forwarding_state, layer_owner_key, owner_name);
        } else {
            /* There is no owner. Check if the current forwarding state of
             * the layer is blocked. */
            if (layer->blocked) {
                /* The forwarding state is blocked. This implies:
                 * - The admin/operator state is down.
                 * - The forwarding state of a previous layer is blocked.
                 *
                 * Remove the key,value pair for the forwarding state of this
                 * layer and the asserting protocol. */
                smap_remove(forwarding_state, layer_key);
                smap_remove(forwarding_state, layer_owner_key);
            } else {
                /* The forwarding state is open. Set the forwarding state as
                 * open for current layer and remove any asserting protocol. */
                smap_replace(forwarding_state, layer_key, state_value);
                smap_remove(forwarding_state, layer_owner_key);
            }
        }

        /* Move to the next forwarding layer in the hierarchy */
        last_layer = layer;
        layer = layer->next;
    }

    /* Set the forwarding state of the interface based on the forwarding state
     * of the last layer.
     * If there isn't one, set the interface state as forwarding. */
    if (last_layer) {
        state_value = intfd_arbiter_get_state_value(last_layer->blocked);
        smap_replace(forwarding_state, INTERFACE_FORWARDING_STATE_MAP_FORWARDING,
                     state_value);
    } else {
        smap_replace(forwarding_state, INTERFACE_FORWARDING_STATE_MAP_FORWARDING,
                     INTERFACE_FORWARDING_STATE_FORWARDING_TRUE);
    }
}

/*!
 * @brief      Function to determine the forwarding state of an interface
 *             from "LACP" perspective.
 *
 * @param[in]  ifrow     The interface for which the arbiter is running.
 *
 * @return     true     If lacp deems the interface should be blocked.
 *             false    If lacp deems the interface should be forwarding.
 */
bool
intfd_arbiter_lacp_state(const struct ovsrec_interface *ifrow)
{
    /* Get the forwarding state for this protocol. */
    const char *bond_status;
    bond_status = smap_get(&ifrow->bond_status, INTERFACE_BOND_STATUS_MAP_STATE);
    if (!bond_status || STR_EQ(bond_status, INTERFACE_BOND_STATUS_UP)) {
        return false;
    } else {
        return true;
    }
}

/*!
 * @brief      Function to register the forwarding layer 'aggregation' and all
 *             the applicable protocols at this layer with the arbiter.
 *
 * @param[in,out]  layer_head     The head of the linked list of forwarding
 *                                layers.
 *
 * @return     Nothing
 */
void
intfd_arbiter_layer_aggregation_register(
        struct intfd_arbiter_layer_class **layer_head)
{
    struct intfd_arbiter_layer_class *aggregation;
    struct intfd_arbiter_proto_class *proto, *proto_head = NULL;

    /* Define and register the aggregation layer */
    aggregation = xzalloc(sizeof(struct intfd_arbiter_layer_class));
    if (aggregation == NULL) {
        VLOG_ERR("Failed to register aggregation layer - "
                 "Memory allocation failed.");
        return;
    }
    aggregation->id = INTERFACE_FORWARDING_STATE_LAYER_AGGREGATION;
    aggregation->owner = INTERFACE_FORWARDING_STATE_PROTO_NONE;
    aggregation->run = intfd_arbiter_layer_run;
    aggregation->next = NULL;
    aggregation->prev = NULL;

    /* Define and register the protocols running at aggregation layer.
     * The ones registered first trumps in precedence over the
     * ones following it. */

    /* Register LACP */
    proto = xzalloc(sizeof(struct intfd_arbiter_proto_class));
    if (proto == NULL) {
        VLOG_ERR("Failed to register proto class for aggregation layer - "
                 "Memory allocation failed.");
        return;
    }
    proto->id = INTERFACE_FORWARDING_STATE_PROTO_LACP;
    proto->run = intfd_arbiter_proto_run;
    proto->get_state = intfd_arbiter_lacp_state;
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
void
intfd_arbiter_init(void)
{
    memset(&intfd_arbiter, 0, sizeof(struct intfd_arbiter_class));

    /* Initialize and attach the interface 'aggregation' layer */
    intfd_arbiter_layer_aggregation_register(&intfd_arbiter.layers);

    return;
}
