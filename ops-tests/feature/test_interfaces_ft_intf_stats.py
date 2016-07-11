# -*- coding: utf-8 -*-
#
# Copyright (C) 2016 Hewlett Packard Enterprise Development LP
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
Author: Mohanraju T - mohan.raj.thangamani@hpe.com
OpenSwitch Test for simple ping between nodes.
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


def test_interfaces_ft_intf_stats(topology):
    """
    Set network addresses and static routes between nodes and ping h2 from h1.
    """
    ops1 = topology.get('ops1')
    hs1 = topology.get('hs1')
    hs2 = topology.get('hs2')
    p1 = ops1.ports['p1']
    p2 = ops1.ports['p2']

    assert ops1 is not None
    assert hs1 is not None
    assert hs2 is not None

    # Configure interfaces 1 and 2 with interface 1 is down
    with ops1.libs.vtysh.ConfigInterface(p1) as ctx:
        ctx.no_routing()
        ctx.no_shutdown()

    with ops1.libs.vtysh.ConfigInterface(p2) as ctx:
        ctx.no_routing()
        ctx.no_shutdown()

    # Configure vlan and switch interfaces
    with ops1.libs.vtysh.ConfigVlan('10') as ctx:
        ctx.no_shutdown()

    with ops1.libs.vtysh.ConfigInterface(p1) as ctx:
        ctx.vlan_access('10')

    with ops1.libs.vtysh.ConfigInterface(p2) as ctx:
        ctx.vlan_access('10')

    result2_tx_base = 0
    result1_rx_base = 0
    # Configure host interfaces
    hs1.libs.ip.interface('1', addr='10.0.10.50/24', up=True)
    hs2.libs.ip.interface('1', addr='10.0.10.60/24', up=True)
    result1 = ops1.libs.vtysh.show_interface(p1)
    result2 = ops1.libs.vtysh.show_interface(p2)
    result1_rx_base = result1['rx_packets']
    result2_tx_base = result2['tx_packets']
    print("The Counter values are: ")
    print(result1_rx_base)
    print(result2_tx_base)
    # Send 5 ping packets
    hs1.libs.ping.ping(5, '10.0.10.60')
    # sleep 5 secs
    sleep(5)
    result1 = ops1.libs.vtysh.show_interface(p1)
    result2 = ops1.libs.vtysh.show_interface(p2)
    print("Show interfaces output are: ")
    print(result1)
    print(result2)
    assert result2['tx_packets'] >= result2_tx_base\
        and result1['rx_packets'] >= result1_rx_base,\
        "The Packets transmitted and received mismatches"
    # Send again 5 ping packets
    hs1.libs.ping.ping(5, '10.0.10.60')
    # sleep 5 secs
    sleep(5)
    result3 = ops1.libs.vtysh.show_interface(p1)
    result4 = ops1.libs.vtysh.show_interface(p2)
    result3_rx_base = result1['rx_packets']
    result4_tx_base = result2['tx_packets']
    print("Show interfaces output are: ")
    print(result3)
    print(result4)
    assert result4['tx_packets'] >= result4_tx_base\
        and result3['rx_packets'] >= result3_rx_base,\
        "The Packets transmitted and received mismatches"
    result1 = ops1.libs.vtysh.show_interface(p1)
    result2 = ops1.libs.vtysh.show_interface(p2)
    assert result4['tx_packets'] >= result4_tx_base\
        and result3['rx_packets'] >= result3_rx_base,\
        "The Packets transmitted and received mismatches"
    print("Show interfaces output are: ")
    print(result1)
    print(result2)
