# Copyright (C) 2016 Hewlett-Packard Development Company, L.P.
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

##########################################################################
# Name:        test_intfd_ct_interface_split_with_lag.py
#
# Objective:   Verify functionality for LAG with split Interfaces
#
# Topology:    1 switch (DUT running OpenSwitch)
#
##########################################################################

from pytest import mark


TOPOLOGY = """
#
#
# +-------+
# |       |
# |  sw1  |
# |       |
# +-------+
#

# Nodes
[type=openswitch name="Switch 1"] sw1

# Links
"""


@mark.gate
def test_lacp_split_interface(topology, step):
    sw1 = topology.get('sw1')

    assert sw1 is not None

    step("Configure LAG in switch")
    with sw1.libs.vtysh.ConfigInterfaceLag("1") as ctx:
        ctx.lacp_mode_active()

    step("Associate interface 50 to lag1")
    with sw1.libs.vtysh.ConfigInterface("50") as ctx:
        ctx.lag("1")

    step("Validate interface 50 was associated to lag1")
    output = sw1.libs.vtysh.show_lacp_aggregates()
    print(output)
    assert '50' in output['lag1']['interfaces']

    # This test is using rapid fire because the Modular Framework
    # it's not allowing to change the prompt to something different
    # than "switch#". There is an enhancement open to fix this
    step("Split interface 50")
    sw1('configure terminal')
    sw1('interface 50')
    sw1._shells['vtysh']._prompt = ('.*Do you want to continue [y/n]?')
    sw1('split')
    sw1._shells['vtysh']._prompt = ('(^|\n)switch(\\([\\-a-zA-Z0-9]*\\))?#')
    sw1('y')
    sw1('exit')
    sw1('exit')

    step("Validate interface 50 was removed from lag1 because is now split")
    output = sw1.libs.vtysh.show_lacp_aggregates()
    assert '50' not in output['lag1']['interfaces']

    step("Associate interface 50-1 to lag1")
    with sw1.libs.vtysh.ConfigInterface("50-1") as ctx:
        ctx.lag("1")

    step("Associate interface 50-2 to lag1")
    with sw1.libs.vtysh.ConfigInterface("50-2") as ctx:
        ctx.lag("1")

    step("Validate interface 50-1 and 50-2 were associated to lag1")
    output = sw1.libs.vtysh.show_lacp_aggregates()
    assert '50-1' in output['lag1']['interfaces']
    assert '50-2' in output['lag1']['interfaces']

    # This test is using rapid fire because the Modular Framework
    # it's not allowing to change the prompt to something different
    # than "switch#". There is an enhancement open to fix this
    step("Unsplit interface 50")
    sw1('configure terminal')
    sw1('interface 50')
    sw1._shells['vtysh']._prompt = ('.*Do you want to continue [y/n]?')
    sw1('no split')
    sw1._shells['vtysh']._prompt = ('(^|\n)switch(\\([\\-a-zA-Z0-9]*\\))?#')
    sw1('y')
    sw1('exit')
    sw1('exit')

    step("Validate interface 50-1 and 50-2 were removed from lag1 because "
         "is not split anymore")
    output = sw1.libs.vtysh.show_lacp_aggregates()
    assert '50-1' not in output['lag1']['interfaces']
    assert '50-2' not in output['lag1']['interfaces']
