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
OpenSwitch Test for simple ping between nodes, when interface enable/disable
"""

from __future__ import unicode_literals, absolute_import
from __future__ import print_function, division

from time import sleep

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


def interface_disable(ops, interface):
    with ops.libs.vtysh.ConfigInterface(interface) as ctx:
        ctx.no_routing()
        ctx.shutdown()


def interface_enable(ops, interface):
    with ops.libs.vtysh.ConfigInterface(interface) as ctx:
        ctx.no_routing()
        ctx.no_shutdown()


def vlan_config(ops, vlan_id):
    with ops.libs.vtysh.ConfigVlan(vlan_id) as ctx:
        ctx.no_shutdown()


def vlan_config_interface(ops, interface, vlan_id):
    with ops.libs.vtysh.ConfigInterface(interface) as ctx:
        ctx.vlan_access(vlan_id)


def ovs_chek(ops, interface):
    cmd = "ovs-vsctl get interface {interface} admin_state".format(**locals())
    admin_state = ops(cmd, shell="bash")
    return admin_state


def test_ft_interface_enable_disable(topology):

    ops1 = topology.get('ops1')
    hs1 = topology.get('hs1')
    hs2 = topology.get('hs2')
    assert ops1 is not None
    assert hs1 is not None
    assert hs2 is not None
    p1 = ops1.ports['p1']
    p2 = ops1.ports['p2']

    # Configure interfaces 1 and 2 with interface 1 is down
    interface_disable(ops1, p1)
    interface_enable(ops1, p2)
    admin_state = ovs_chek(ops1, p1)
    assert admin_state == 'down'

    # Configure vlan and switch interfaces
    vlan_config(ops1, '10')
    vlan_config_interface(ops1, p1, '10')
    vlan_config_interface(ops1, p2, '10')

    # Configure host interfaces
    hs1.libs.ip.interface('1', addr=hs1ip+"/"+mask, up=True)
    hs2.libs.ip.interface('1', addr=hs2ip+"/"+mask, up=True)
    sleep(5)

    # Test ping
    ping = hs1.libs.ping.ping(1, hs2ip)
    assert ping['received'] == 0

    # Enable interface 1 and verify that ping succeeds :
    interface_enable(ops1, p1)
    admin_state = ovs_chek(ops1, p1)
    assert admin_state == 'up'
    sleep(5)

    # Test ping
    ping = hs1.libs.ping.ping(1, hs2ip)
    assert ping['transmitted'] == ping['received'] == 1

    # Disable interface 1 and verify that ping succeeds :
    interface_disable(ops1, p1)
    admin_state = ovs_chek(ops1, p1)
    assert admin_state == 'down'
    sleep(5)

    # Test ping
    ping = hs1.libs.ping.ping(1, hs2ip)
    assert ping['received'] == 0
