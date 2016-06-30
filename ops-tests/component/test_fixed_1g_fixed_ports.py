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
OpenSwitch Test for interface related configurations.
"""

from pytest import mark
from time import sleep
TOPOLOGY = """
# +-------+
# |  ops1 |
# +-------+

# Nodes
[type=openswitch name="OpenSwitch 1"] ops1
"""


fixed_intf = "1"
test_intf = "21"
split_parent = '50'
split_children = ['50-1', '50-2', '50-3', '50-4']
qsfp_intf = "53"


def sw_set_intf_user_config(dut, int, conf):
    c = "set interface {int}".format(int=str(int))
    for s in conf:
        c += " user_config:{s}".format(s=s)
    return dut(c, shell="vsctl")


def sw_clear_user_config(dut, int):
    return dut("clear interface {int} user_config".format(int=str(int)),
               shell="vsctl")


def sw_set_intf_pm_info(dut, int, conf):
    c = "set interface {int}".format(int=str(int))
    for s in conf:
        c += " pm_info:{s}".format(s=s)
    return dut(c, shell="vsctl")


def sw_get_intf_state(dut, int, fields):
    c = "get interface {int}".format(int=str(int))
    for f in fields:
        c += " {f}".format(f=f)
    out = dut(c, shell="vsctl").splitlines()
    # If a single column value is requested,
    # then return a singleton value instead of list.
    if len(out) == 1:
        out = out[0]
    return out


def short_sleep(tm=.5):
    sleep(tm)


@mark.gate
def test_fixed_1g_fixed_ports(topology, step):
    ops1 = topology.get("ops1")
    assert ops1 is not None

    ops1("/bin/systemctl stop ops-pmd", shell="bash")

    step("Step 1- Configure interface fixed_intf on switch s1 as no routing "
         "else the interface will use the port admin logic to set its "
         "own hw_intf_config state")
    ops1("configure terminal")
    ops1("interface {int}".format(int=fixed_intf))
    ops1("no routing")
    ops1("end")

    step("Step 2- In VSI environment interfaces 1 - 10 are supposed to be "
         "fixed, without any pluggable modules. If not skip this test.")
    if topology.engine == 'docker':
        is_pluggable, connector = sw_get_intf_state(ops1, fixed_intf,
                                                    ['hw_intf_info:pluggable',
                                                     'hw_intf_info:connector'])
        assert is_pluggable == '"false"' and connector == '"RJ45"'

    step("Step 3- The default state of these interfaces should be "
         "'admin_down'")
    err = sw_get_intf_state(ops1, fixed_intf, ['error'])
    assert err == 'admin_down'

    step("Step 4- Enable the interface it should come up.")
    sw_set_intf_user_config(ops1, fixed_intf, ['admin=up'])

    array_values = ['hw_intf_config:enable',
                    'hw_intf_config:autoneg',
                    'hw_intf_config:interface_type']
    if topology.engine == 'docker':
        hw_enable, autoneg, intf_type = sw_get_intf_state(ops1,
                                                          fixed_intf,
                                                          array_values)
        assert (
            hw_enable == '"true"' and autoneg == 'on' and
            intf_type == '"1GBASE_T"'
        )
