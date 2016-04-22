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
def test_pm_info_detect(topology, step):
    ops1 = topology.get("ops1")
    assert ops1 is not None

    ops1("/bin/systemctl stop ops-pmd", shell="bash")

    step("Step 1- Enable the interface, and set the pm_info to valid values.")
    sw_clear_user_config(ops1, test_intf)
    sw_set_intf_user_config(ops1, test_intf, ['admin=up'])
    sw_set_intf_pm_info(ops1, test_intf, ('connector=SFP_RJ45',
                                          'connector_status=supported'))
    short_sleep()

    step("Step 2- Make sure that 'error' status is not 'module_missing'")
    err = sw_get_intf_state(ops1, test_intf, ['error'])
    assert 'module_missing' != err

    step("Step 3- Set pm_info:connector as 'absent'")
    sw_set_intf_pm_info(ops1, test_intf, ('connector=absent',
                                          'connector_status=unrecognized'))
    short_sleep()

    step("Step 4- Make sure that 'error' status is set to 'module_missing'")
    err, hw_enable = sw_get_intf_state(ops1, test_intf,
                                       ['error', 'hw_intf_config:enable'])
    assert err == 'module_missing' or hw_enable == '"false"'

    step("Step 5- Set pm_info:connector as 'unknown' and "
         "connector_status=unrecognized")
    sw_set_intf_pm_info(ops1, test_intf, ('connector=unknown',
                                          'connector_status=unrecognized'))
    short_sleep()

    err, hw_enable = sw_get_intf_state(ops1, test_intf,
                                       ['error', 'hw_intf_config:enable'])
    assert err == 'module_unrecognized' and hw_enable == '"false"'

    step("Step 6- Set pm_info:connector as 'unknown' and "
         "connector_status=unsupported")
    sw_set_intf_pm_info(ops1, test_intf, ('connector=unknown',
                                          'connector_status=unsupported'))
    short_sleep()

    err, hw_enable = sw_get_intf_state(ops1, test_intf,
                                       ['error', 'hw_intf_config:enable'])
    assert err == 'module_unsupported' and hw_enable == '"false"'

    step("Step 7- Set pm_info:connector as <unknown to intfd, such as SFP_LX>"
         " and connector_status=supported")
    sw_set_intf_pm_info(ops1, test_intf, ('connector=SFP_LX',
                                          'connector_status=supported'))
    short_sleep()

    err, hw_enable = sw_get_intf_state(ops1, test_intf,
                                       ['error', 'hw_intf_config:enable'])
    assert err == 'module_unsupported' and hw_enable == '"false"'

    step("Step 8- Verify the hw_intf_config:interface_type based on "
         "'pm_info:connector'")
    pm_intf_type = {
        'SFP_RJ45': '"1GBASE_T"',
        'SFP_SX': '"1GBASE_SX"',
        'SFP_SR': '"10GBASE_SR"',
        'SFP_LR': '"10GBASE_LR"',
        'SFP_DAC': '"10GBASE_CR"'
    }

    for pm_type, intf_type in pm_intf_type.items():
        sw_set_intf_pm_info(ops1, test_intf,
                            ['connector={pm}'.format(pm=pm_type),
                             'connector_status=supported'])
        short_sleep()

        out = sw_get_intf_state(ops1, test_intf,
                                ['hw_intf_config:interface_type'])
        assert intf_type == out

    step("Step 9- Configure interface on switch s1 as no routing else the "
         "interface will use the port admin logic to set its own "
         "hw_intf_config state")
    ops1("configure terminal")
    ops1("interface {int}".format(int=split_parent))
    ops1("no routing")
    ops1("end")

    step("Step 10- In X86 config Interface 50 is Splittable port."
         "Enable the split parent interface.")
    sw_set_intf_user_config(ops1, split_parent,
                            ['admin=up', 'lane_split=no-split'])

    pm_intf_type = {
        'SFP_SR': '"10GBASE_SR"',
        'SFP_LR': '"10GBASE_LR"',
        'SFP_DAC': '"10GBASE_CR"',
        'QSFP_CR4': '"40GBASE_CR4"',
        'QSFP_SR4': '"40GBASE_SR4"',
        'QSFP_LR4': '"40GBASE_LR4"'
    }

    for pm_type, intf_type in pm_intf_type.items():
        sw_set_intf_pm_info(ops1, split_parent,
                            ['connector={pm}'.format(pm=pm_type),
                             'connector_status=supported'])
        short_sleep()

        out = sw_get_intf_state(ops1, split_parent,
                                ['hw_intf_config:interface_type'])
        assert intf_type == out

    step("Step 11- Enable the split children port.")
    sw_set_intf_user_config(ops1, split_parent,
                            ['admin=down', 'lane_split=split'])
    for child_port in split_children:
        ops1("configure terminal")
        ops1("interface {port}".format(port=child_port))
        ops1("no routing")
        ops1("end")
        sw_set_intf_user_config(ops1, child_port, ['admin=up'])

    pm_intf_type = {
        'QSFP_CR4': '"10GBASE_CR"',
        'QSFP_SR4': '"10GBASE_SR"',
        'QSFP_LR4': '"10GBASE_LR"'
    }

    for pm_type, intf_type in pm_intf_type.items():
        sw_set_intf_pm_info(ops1, split_parent,
                            ['connector={pm}'.format(pm=pm_type),
                             'connector_status=supported'])
        short_sleep()

        for child_port in split_children:
            out = sw_get_intf_state(ops1, child_port,
                                    ['hw_intf_config:interface_type'])
            assert intf_type == out

    step("Step 12- Unsupported pluggable modules when interface is split")
    pm_intf_type = ['SFP_RJ45', 'SFP_SX', 'SFP_SR', 'SFP_LR', 'SFP_DAC']

    for pm_type in pm_intf_type:
        sw_set_intf_pm_info(ops1, split_parent,
                            ['connector='.format(pm=pm_type),
                             'connector_status=supported'])
        short_sleep()

        for child_port in split_children:
            err, hw_enable = sw_get_intf_state(ops1, child_port,
                                               ['error',
                                                'hw_intf_config:enable'])
            assert err == 'module_unsupported' and hw_enable == '"false"'

    step("Step 13- Clear the OVSDB config of the interfaces")
    sw_clear_user_config(ops1, test_intf)
    sw_set_intf_pm_info(ops1, test_intf, ('connector=absent',
                                          'connector_status=unsupported'))

    sw_clear_user_config(ops1, split_parent)
    sw_set_intf_pm_info(ops1, split_parent, ('connector=absent',
                                             'connector_status=unsupported'))

    for child_port in split_children:
        sw_clear_user_config(ops1, child_port)
