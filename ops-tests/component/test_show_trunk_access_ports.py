####################################################################################
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
###################################################################################

'''
This file conatains component tests for following commands:
    1. show interface trunk
    2. show interface access
'''

from time import sleep

TOPOLOGY = '''
# +-------+
# |  ops1 |
# +-------+

# Nodes
[type=openswitch name="OpenSwitch 1"] ops1
'''


def configure_switch(sw):
    sw("configure terminal")

    ''' configure vlans '''
    sw("vlan 2")
    sw("vlan 3")
    sw("vlan 4")
    sw("vlan 12")
    sw("vlan 13")
    sw("vlan 14")
    sw("vlan 15")

    ''' configure interfaces '''
    sw("interface 1")
    sw("no shutdown")
    sw("no routing")
    sw("vlan access 12")

    sw("interface 2")
    sw("no shutdown")
    sw("no routing")
    sw("vlan access 13")

    sw("interface 3")
    sw("no routing")

    sw("interface 4")
    sw("no routing")
    sw("vlan trunk allowed 12-14")

    sw("interface 5")
    sw("no routing")
    sw("vlan trunk allowed 2-4")
    sw("vlan trunk native 2")

    sw("interface 7")
    sw("no routing")
    sw("vlan trunk allowed 12-14")
    sw("vlan trunk native tag")

    sw("end")


def show_interface_trunk_verification(sw):
    out = sw("show interface trunk")
    lines = out.split('\n')
    count = 0
    for line in lines:
        if '4' in line and '1' in line and 'trunk' in line and 'down' in line:
            count = count + 1
        elif '5' in line and '2' in line and 'native-untagged' in line and 'down' in line:
            count = count + 1
        elif '7' in line and '1' in line and 'native-tagged' in line and 'down' in line:
            count = count + 1
        elif '4' in line and '12, 13, 14' in line:
            count = count + 1
        elif '5' in line and '2, 3, 4' in line:
            count = count + 1
        elif '7' in line and '2, 3, 4' in line:
            count = count + 1

    assert count == 6, \
            "show interface trunk command - FAILED"


def show_interface_access_verification(sw):
    out = sw("show interface access")
    lines = out.split('\n')
    count = 0
    for line in lines:
        if '1' in line and '12' in line and 'access' in line and 'up' in line:
            count = count + 1
        elif'2' in line and '13' in line and 'access' in line and 'up' in line:
            count = count + 1
        elif '3' in line and '1' in line and 'access' in line and 'down' in line:
            count = count + 1

    assert count == 3, \
            'show interface access command - FAILED'


def test_show_trunk_access_ports(topology, step):
    ops1 = topology.get("ops1")
    assert ops1 is not None

    ops1("/bin/systemctl stop ops-pmd", shell="bash")

    step("Configuring switch")
    configure_switch(ops1)
    sleep(15)

    step("Verify the result of show interface trunk command")
    show_interface_trunk_verification(ops1)

    step("Verify the result of show interface access command")
    show_interface_access_verification(ops1)
