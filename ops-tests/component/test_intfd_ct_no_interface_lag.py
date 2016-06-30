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
OpenSwitch Test for 'no interface lagXX'
That command should display an error message
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
def test_no_interface_lagxx(topology):
    # Testing that 'no interface lagXX' shows an error message
    test_lag = '10'
    ops1 = topology.get('ops1')

    assert ops1 is not None, 'Topology failed getting ops1'

    with ops1.libs.vtysh.ConfigInterfaceLag(test_lag) as ctx:
        ctx.no_routing()

    ops1('configure terminal')
    output = ops1('no interface lag{test_lag}'.format(**locals()))
    ops1('end')

    assert 'Could not remove lag{test_lag}'.format(**locals()) in output

    # Verifying that 'no interface lagXX' did not remove lag XX
    output = ops1('show running-config interface')

    assert 'interface lag {test_lag}'.format(**locals()) in output
