# -*- coding: utf-8 -*-
#
# Copyright (C) 2015-2016 Hewlett Packard Enterprise Development LP
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

"""
OpenSwitch Test for autonegotiation in interface.
"""

from __future__ import unicode_literals, absolute_import
from __future__ import print_function, division
from time import sleep
from pytest import mark

TOPOLOGY = """
# +-------+                    +-------+
# |       |     +--------+     |       |
# |  hs1  <----->  ops1  <----->  hs2  |
# |       |     +--------+     |       |
# +-------+                    +-------+

# Nodes
[type=openswitch name="OpenSwitch 1"] ops1
[type=host name="Host 1"] hs1
[type=host name="Host 2"] hs2

# Links
hs1:1 -- ops1:p1
ops1:p2 -- hs2:1
"""

hs1ip = '10.0.10.20'
hs2ip = '10.0.10.30'
mask = '24'


# COnfigure interface
def config_interface(ops1, interface):
    with ops1.libs.vtysh.ConfigInterface(interface) as ctx:
        ctx.no_routing()
        ctx.no_shutdown()


# Configure vlan switch
def vlan_config(ops1, vlan_id):
    with ops1.libs.vtysh.ConfigVlan(vlan_id) as ctx:
        ctx.no_shutdown()


# COnfig VLAN in switch interface
def vlan_config_interface(ops1, interface, vlan_id):
    with ops1.libs.vtysh.ConfigInterface(interface) as ctx:
        ctx.vlan_access(vlan_id)


# Verifying Autonegotiation state :
def autoneg_chek(ops1, interface):
    cmd = "ovs-vsctl get interface {interface} hw_intf_config:autoneg" \
        .format(**locals())
    autoneg_state = ops1(cmd, shell="bash")
    return autoneg_state


# Verify Auto-Negotiation error
def autoneg_error(ops1, interface):
    cmd = "ovs-vsctl get interface {interface} error".format(**locals())
    autoneg_err = ops1(cmd, shell="bash")
    return autoneg_err


# Turn off auto negotiation
def autoneg_off(ops1, interface):
    with ops1.libs.vtysh.ConfigInterface(interface) as ctx:
        ctx.autonegotiation_off()


# Turn On Auto negotiation
def autoneg_on(ops1, interface):
    with ops1.libs.vtysh.ConfigInterface(interface) as ctx:
        ctx.autonegotiation_on()


# Disable Auto negotiation
def autoneg_disable(ops1, interface):
    with ops1.libs.vtysh.ConfigInterface(interface) as ctx:
        ctx.no_autonegotiation()


# Verify Admin status in bash prompt
def admin_status(ops1, interface):
    cmd = "ovs-vsctl get interface {interface} admin-state".format(**locals())
    admin_state = ops1(cmd, shell="bash")
    return admin_state


@mark.platform_incompatible(['docker'])
def test_ft_interface_intf_autoneg(topology):
    """
    Set network addresses and static routes between nodes and ping h2 from h1.
    """
    ops1 = topology.get('ops1')
    hs1 = topology.get('hs1')
    hs2 = topology.get('hs2')
    assert ops1 is not None
    assert hs1 is not None
    assert hs2 is not None
    p1 = ops1.ports['p1']
    p2 = ops1.ports['p2']

    # Config interfaces
    config_interface(ops1, p1)
    config_interface(ops1, p2)

    # Config VLAN
    vlan_config(ops1, '10')
    vlan_config_interface(ops1, p1, '10')
    vlan_config_interface(ops1, p2, '10')

    # Verify Autoneg :
    autoneg_state = autoneg_chek(ops1, p1)
    assert autoneg_state == 'on'

    # Configure host interfaces
    hs1.libs.ip.interface('1', addr=hs1ip+"/"+mask, up=True)
    hs2.libs.ip.interface('1', addr=hs2ip+"/"+mask, up=True)

    # Test ping
    ping = hs1.libs.ping.ping(1, hs2ip)
    assert ping['transmitted'] == ping['received'] == 1

    # Turning off auto-negotiation:
    autoneg_off(ops1, p1)

    # Verifying Autonegotiation state :
    autoneg_err = autoneg_error(ops1, p1)
    assert autoneg_err == 'autoneg_required'
    sleep(5)

    # Verifying Admin state when auto-negotiation is off :
    admin_state = admin_status(ops1, p1)
    assert admin_state == 'down'
    sleep(5)

    # Test Ping
    ping = hs1.libs.ping.ping(1, hs2ip)
    assert ping['received'] == 0

    # Turn Auto neogtiation on
    autoneg_on(ops1, p1)

    # Verifying Autonegotiation state :
    autoneg_state = autoneg_chek(ops1, p1)
    assert autoneg_state == 'on'
    sleep(5)

    # Verifying Admin state when auto-negotiation is off :
    admin_state = admin_status(ops1, p1)
    assert admin_state == 'up'
    sleep(5)

    # Test Ping
    ping = hs1.libs.ping.ping(1, hs2ip)
    assert ping['transmitted'] == ping['received'] == 1

    # Disable Auto neogtiation
    autoneg_disable(ops1, p1)

    # Verifying Autonegotiation state :
    autoneg_state = autoneg_chek(ops1, p1)
    assert autoneg_state == 'on'
    sleep(5)

    # Verifying Admin state when auto-negotiation is off :
    admin_state = admin_status(ops1, p1)
    assert admin_state == 'up'
    sleep(5)

    # Test Ping
    ping = hs1.libs.ping.ping(1, hs2ip)
    assert ping['transmitted'] == ping['received'] == 1
