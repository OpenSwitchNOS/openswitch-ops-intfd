# (C) Copyright 2016 Hewlett Packard Enterprise Development LP
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.
#
##########################################################################

"""
OpenSwitch Test for LAG interface related configuration
"""
from pytest import mark

TOPOLOGY = """
# +-------+
# |  ops1 |
# +-------+

# Nodes
[type=openswitch name="OpenSwitch 1"] ops1

# Links
"""


@mark.gate
def test_show_interface_lag_(topology, step):
    ops1 = topology.get('ops1')

    assert ops1 is not None

    step("Test show interface lag command")
    with ops1.libs.vtysh.ConfigInterfaceLag("1") as ctx:
        ctx.ip_address("10.1.1.1/24")
        ctx.ip_address_secondary("20.1.1.2/24")
        ctx.ipv6_address("2001::1/12")
        ctx.ipv6_address_secondary("3001::2/12")

    output = ops1.libs.vtysh.show_interface("lag1")
    assert output['ipv4'] == '10.1.1.1/24',\
        "LAG IP address is not properly configured - Failed"
    assert output['ipv4_secondary'] == '20.1.1.2/24',\
        "LAG IP secondary address is not properly configured - Failed"
    assert output['ipv6'] == '2001::1/12',\
        "LAG IPv6 address is not properly configured - Failed"
    assert output['ipv6_secondary'] == '3001::2/12',\
        "LAG IPv6 secondary address is not properly configured - Failed"


def test_show_ip_interface_lag_(topology, step):
    ops1 = topology.get('ops1')

    assert ops1 is not None

    step("Test show ip interface lag command")
    with ops1.libs.vtysh.ConfigInterfaceLag("2") as ctx:
        ctx.ip_address("30.1.1.1/24")
        ctx.ip_address_secondary("40.1.1.2/24")
        ctx.ipv6_address("4001::1/12")
        ctx.ipv6_address_secondary("5001::2/12")

    output = ops1.libs.vtysh.show_ip_interface("lag2")
    assert output['ipv4'] == '30.1.1.1/24',\
        "LAG IP address is not properly configured - Failed"
    assert output['ipv4_secondary'] == '40.1.1.2/24',\
        "LAG IP secondary address is not properly configured - Failed"

    output = ops1.libs.vtysh.show_ipv6_interface("lag2")
    assert output['ipv6'] == '4001::1/12',\
        "LAG IPv6 address is not properly configured - Failed"
    assert output['ipv6_secondary'] == '5001::2/12',\
        "LAG IPv6 secondary address is not properly configured - Failed"
