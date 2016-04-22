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


@mark.skipif(True, reason="After interface is supposed to be up, "
                          "hw_intf_config database field doesn't refresh")
def test_user_config_qsfp_splitter(topology, step):
    ops1 = topology.get("ops1")
    assert ops1 is not None

    ops1("/bin/systemctl stop ops-pmd", shell="bash")

    step("Step 1- Verify that the default state of parent interface is"
         " 'admin_down'")
    sw_clear_user_config(ops1, split_parent)
    out = sw_get_intf_state(ops1, split_parent, ['error'])
    assert 'admin_down' == out

    step("Step 2- Verify that the default state of children ports is "
         "'lanes_not_split'")
    for child_port in split_children:
        out = sw_get_intf_state(ops1, child_port, ['error'])
        assert 'lanes_not_split' == out

    step("Step 3- Set Pluggable module info to valid values on parent "
         "interface.")
    sw_set_intf_pm_info(ops1, split_parent, ('connector=QSFP_CR4',
                                             'connector_status=supported'))

    step("Step 4- Make sure that split children are disabled as long as"
         " user_config:lane_split=no-split on the parent interface.")
    sw_set_intf_user_config(ops1, split_parent, ['admin=up',
                                                 'lane_split=no-split'])
    hw_enable = sw_get_intf_state(ops1, split_parent,
                                  ['hw_intf_config:enable'])
    assert hw_enable == '"true"'

    step("Step 5- Enable the split children via user_config, but they should"
         " be still disabled in the hw_intf_config.")
    for child_port in split_children:
        sw_set_intf_user_config(ops1, child_port, ['admin=up'])

    short_sleep()

    for child_port in split_children:
        hw_enable = sw_get_intf_state(ops1, child_port,
                                      ['hw_intf_config:enable'])
        assert hw_enable == '"false"'

    step("Step 6- Split the parent interface and make sure that parent "
         "interface is disabled, and split child ports are enabled.")
    sw_set_intf_user_config(ops1, split_parent, ['admin=up',
                                                 'lane_split=split'])
    short_sleep()

    err, hw_enable = sw_get_intf_state(ops1, split_parent,
                                       ['error',
                                        'hw_intf_config:enable'])
    assert err == 'lanes_split' and hw_enable == '"false"'

    for child_port in split_children:
        hw_enable = sw_get_intf_state(ops1, child_port,
                                      ['hw_intf_config:enable'])
        assert hw_enable == '"true"'

    sw_set_intf_user_config(ops1, split_parent, ['admin=down'])

    for child_port in split_children:
        hw_enable = sw_get_intf_state(ops1, child_port,
                                      ['hw_intf_config:enable'])
        assert hw_enable == '"true"'

    step("Step 7- Change the lane_split to 'no-split', then all child ports "
         "should be disabled.")
    sw_set_intf_user_config(ops1, split_parent, ['lane_split=no-split'])
    for child_port in split_children:
        hw_enable = sw_get_intf_state(ops1, child_port,
                                      ['hw_intf_config:enable'])
        assert hw_enable == '"false"'

    step("Step 8- Cleanup")
    sw_clear_user_config(ops1, split_parent)
    sw_set_intf_pm_info(ops1, split_parent, ('connector=absent',
                                             'connector_status=unsupported'))

    for child_port in split_children:
        sw_clear_user_config(ops1, child_port)
