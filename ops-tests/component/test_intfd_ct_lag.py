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
def test_show_running_config_lag_interface(topology, step):
    ops1 = topology.get('ops1')

    assert ops1 is not None

    step("Test show running-config interface LAG command")
    with ops1.libs.vtysh.ConfigInterfaceLag("1") as ctx:
        ctx.ip_address("10.1.1.1/24")
        ctx.ipv6_address("2001::1/12")
        ''' Fallback all active mode is not implemented yet'''
        # ctx.lacp_fallback_mode_all_active()
        ctx.lacp_fallback_timeout(200)

    output = ops1("show running-config interface")
    lines = output.split('\n')

    success = 0
    for line in lines:
        if 'ip address 10.1.1.1/24' in line:
            success += 1
        if 'ipv6 address 2001::1/12' in line:
            success += 1
        # if 'lacp fallback mode all_active' in line:
        #     success += 1
        if 'lacp fallback timeout 200' in line:
            success += 1
    assert success == 3,\
        'Test show running-config interface command - Failed'

    output = ops1("show running-config interface lag1")
    lines = output.split('\n')
    success = 0
    for line in lines:
        if 'ip address 10.1.1.1/24' in line:
            success += 1
        if 'ipv6 address 2001::1/12' in line:
            success += 1
        # if 'lacp fallback mode all_active' in line:
        #     success += 1
        if 'lacp fallback timeout 200' in line:
            success += 1
    assert success == 3,\
        'Test show running-config interface <lag_id> - Failed'
