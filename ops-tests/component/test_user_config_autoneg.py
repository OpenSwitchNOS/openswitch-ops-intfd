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
                          "hw_intf_config:auton field is not present"
                          " in the ovsdb")
def test_user_config_autoneg(topology, step):
    ops1 = topology.get("ops1")
    assert ops1 is not None

    ops1("/bin/systemctl stop ops-pmd", shell="bash")

    step("Step 1- Intfd user config autoneg tests")
    sw_clear_user_config(ops1, test_intf)
    sw_clear_user_config(ops1, split_parent)
    sw_set_intf_user_config(ops1, test_intf, ['admin=up'])
    sw_set_intf_user_config(ops1, split_parent, ['admin=up',
                                                 'lane_split=no-split'])

    pm_autoneg = {
        'SFP_RJ45': 'on',
        'SFP_SX': 'on',
        'SFP_SR': 'off',
        'SFP_LR': 'off',
        'SFP_DAC': 'off'
    }

    for pm_type, autoneg in pm_autoneg.items():
        sw_set_intf_pm_info(ops1, test_intf,
                            ["connector={pm}".format(pm=pm_type),
                             "connector_status=supported"])
        short_sleep()

        out = sw_get_intf_state(ops1, test_intf, ['hw_intf_config:autoneg'])
        assert autoneg == out

    pm_autoneg = {
        'SFP_RJ45': 'on',
        'SFP_SR': 'off',
        'SFP_LR': 'off',
        'SFP_DAC': 'off',
        'QSFP_CR4': 'on',
        'QSFP_SR4': 'off',
        'QSFP_LR4': 'off'
    }

    for pm_type, autoneg in pm_autoneg.items():
        sw_set_intf_pm_info(ops1, split_parent,
                            ['connector={pm}'.format(pm=pm_type),
                             'connector_status=supported'])
        short_sleep()

        out = sw_get_intf_state(ops1, split_parent,
                                ['hw_intf_config:autoneg'])
        assert autoneg == out

    step("Step 2- Enable the split children port.")
    sw_set_intf_user_config(ops1, split_parent, ['admin=down',
                                                 'lane_split=split'])
    for child_port in split_children:
        sw_set_intf_user_config(ops1, child_port, ['admin=up'])

    step("Step 3- HW intf config for split children.")
    pm_autoneg = {
        'QSFP_CR4': 'off',
        'QSFP_SR4': 'off',
        'QSFP_LR4': 'off'
    }

    for pm_type, autoneg in pm_autoneg.items():
        sw_set_intf_pm_info(ops1, split_parent,
                            ['connector=' + pm_type,
                             'connector_status=supported'])
        short_sleep()

        for child_port in split_children:
            out = sw_get_intf_state(ops1, child_port,
                                    ['hw_intf_config:autoneg'])
            assert autoneg == out

    step("Step 4- Test the interaction of user input for speeds and AN for"
         " the various interface types to ensure AN and speeds are properly "
         "set.")
    sw_clear_user_config(ops1, test_intf)
    sw_clear_user_config(ops1, qsfp_intf)

    sfp_fixed = '"1000"'
    qsfp_fixed = '"40000"'

    step("Step 5- if AN supported or required: AN=true, "
         "user_speeds=supported_speeds")
    sw_set_intf_pm_info(ops1, test_intf, ('connector=SFP_RJ45',
                                          'connector_status=supported'))
    short_sleep()
    sw_set_intf_user_config(ops1, test_intf, ['admin=up'])
    short_sleep()

    autoneg, speeds = sw_get_intf_state(ops1, test_intf,
                                        ['hw_intf_config:autoneg',
                                         'hw_intf_config:speeds'])

    assert autoneg == 'on' and speeds == sfp_fixed

    step("Step 6- if AN not_supported: AN=false, speeds=fixed_speed")
    sw_set_intf_pm_info(ops1, qsfp_intf, ('connector=QSFP_SR4',
                                          'connector_status=supported'))
    short_sleep()

    step("Step 7- Configure interface on switch s1 as no routing else the "
         "interface will use the port admin logic to set its own "
         "hw_intf_config state")
    ops1("configure terminal")
    ops1("interface {int}".format(int=qsfp_intf))
    ops1("no routing")
    ops1("end")
    sw_set_intf_user_config(ops1, qsfp_intf, ['admin=up'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, qsfp_intf, ['hw_intf_config:enable'])
    assert hw_enable == "\"true\""

    autoneg, speeds = sw_get_intf_state(ops1, qsfp_intf,
                                        ['hw_intf_config:autoneg',
                                         'hw_intf_config:speeds'])

    assert autoneg == 'off' and speeds == qsfp_fixed

    step("Step 8- if AN supported or required: AN=true, "
         "speeds=supported_speeds")
    sw_set_intf_pm_info(ops1, test_intf, ('connector=SFP_RJ45',
                                          'connector_status=supported'))
    short_sleep()
    sw_set_intf_user_config(ops1, test_intf, ['admin=up', 'autoneg=on'])
    short_sleep()

    autoneg, speeds = sw_get_intf_state(ops1, test_intf,
                                        ['hw_intf_config:autoneg',
                                         'hw_intf_config:speeds'])

    assert autoneg == 'on' and speeds == sfp_fixed

    step("Step 9- if AN not_supported: user error! \"AN not supported\"")
    sw_set_intf_pm_info(ops1, qsfp_intf, ('connector=QSFP_SR4',
                                          'connector_status=supported'))
    short_sleep()
    sw_set_intf_user_config(ops1, qsfp_intf, ['admin=up', 'autoneg=on'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, qsfp_intf, ['hw_intf_config:enable'])
    assert hw_enable == "\"false\""
    error = sw_get_intf_state(ops1, qsfp_intf, ['error'])
    assert error == "autoneg_not_supported"

    step("Step 10- if AN required: user error! \"AN required\"")
    sw_set_intf_pm_info(ops1, test_intf, ('connector=SFP_RJ45',
                                          'connector_status=supported'))
    short_sleep()
    sw_set_intf_user_config(ops1, test_intf, ['admin=up', 'autoneg=off'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, test_intf, ['hw_intf_config:enable'])
    assert hw_enable == "\"false\""
    error = sw_get_intf_state(ops1, test_intf, ['error'])
    assert error == "autoneg_required"

    step("Step 11- if AN not_supported: AN=false, speeds=fixed_speed")
    sw_set_intf_pm_info(ops1, qsfp_intf, ('connector=QSFP_LR4',
                                          'connector_status=supported'))
    short_sleep()
    sw_set_intf_user_config(ops1, qsfp_intf, ['admin=up', 'autoneg=off'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, qsfp_intf, ['hw_intf_config:enable'])
    assert hw_enable == "\"true\""

    autoneg, speeds = sw_get_intf_state(ops1, qsfp_intf,
                                        ['hw_intf_config:autoneg',
                                         'hw_intf_config:speeds'])

    assert autoneg == 'off' and speeds == qsfp_fixed

    step("Step 12- if AN supported or required: AN=true, speeds=cust_speeds")
    sw_clear_user_config(ops1, test_intf)
    short_sleep()
    sw_set_intf_pm_info(ops1, test_intf, ('connector=SFP_RJ45',
                                          'connector_status=supported'))
    short_sleep()
    sw_set_intf_user_config(ops1, test_intf, ['admin=up', 'speeds=1000'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, test_intf, ['hw_intf_config:enable'])
    assert hw_enable == "\"true\""

    autoneg, speeds = sw_get_intf_state(ops1, test_intf,
                                        ['hw_intf_config:autoneg',
                                         'hw_intf_config:speeds'])

    assert autoneg == 'on' and speeds == '"1000"'

    step("Step 13- if AN not_supported: AN=false, speeds=first_speed")
    sw_clear_user_config(ops1, qsfp_intf)
    short_sleep()
    sw_set_intf_pm_info(ops1, qsfp_intf, ('connector=QSFP_LR4',
                                          'connector_status=supported'))
    short_sleep()
    sw_set_intf_user_config(ops1, qsfp_intf, ['admin=up', 'speeds=40000'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, qsfp_intf, ['hw_intf_config:enable'])
    assert hw_enable == "\"true\""

    autoneg, speeds = sw_get_intf_state(ops1, qsfp_intf,
                                        ['hw_intf_config:autoneg',
                                         'hw_intf_config:speeds'])

    assert autoneg == 'off' and speeds == qsfp_fixed

    step("Step 14- if AN supported or required: AN=true, speeds=cust_speeds")
    short_sleep()
    sw_set_intf_pm_info(ops1, test_intf, ('connector=SFP_RJ45',
                                          'connector_status=supported'))
    short_sleep()
    sw_set_intf_user_config(ops1, test_intf, ['admin=up', 'autoneg=on',
                                              'speeds=1000'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, test_intf, ['hw_intf_config:enable'])
    assert hw_enable == "\"true\""

    autoneg, speeds = sw_get_intf_state(ops1, test_intf,
                                        ['hw_intf_config:autoneg',
                                         'hw_intf_config:speeds'])

    assert autoneg == 'on' and speeds == '"1000"'

    step("Step 15- if AN not_supported: user error! \"AN not supported\"")
    short_sleep()
    sw_set_intf_pm_info(ops1, qsfp_intf, ('connector=QSFP_LR4',
                                          'connector_status=supported'))
    short_sleep()
    sw_set_intf_user_config(ops1, qsfp_intf, ['admin=up', 'autoneg=on',
                                              'speeds=40000'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, qsfp_intf, ['hw_intf_config:enable'])
    assert hw_enable == "\"false\""
    error = sw_get_intf_state(ops1, qsfp_intf, ['error'])
    assert error == "autoneg_not_supported"

    step("Step 16- if AN required: user error! \"AN required\"")
    sw_set_intf_pm_info(ops1, test_intf, ('connector=SFP_RJ45',
                                          'connector_status=supported'))
    short_sleep()
    sw_set_intf_user_config(ops1, test_intf, ['admin=up', 'autoneg=off',
                                              'speeds=1000'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, test_intf, ['hw_intf_config:enable'])
    assert hw_enable == "\"false\""
    error = sw_get_intf_state(ops1, test_intf, ['error'])
    assert error == "autoneg_required"

    step("Step 17- if AN not_supported: AN=false, speeds=first_speed")
    short_sleep()
    sw_set_intf_pm_info(ops1, qsfp_intf, ('connector=QSFP_LR4',
                                          'connector_status=supported'))
    short_sleep()
    sw_set_intf_user_config(ops1, qsfp_intf, ['admin=up', 'autoneg=off',
                                              'speeds=40000'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, qsfp_intf, ['hw_intf_config:enable'])
    assert hw_enable == "\"true\""

    autoneg, speeds = sw_get_intf_state(ops1, qsfp_intf,
                                        ['hw_intf_config:autoneg',
                                         'hw_intf_config:speeds'])

    assert autoneg == 'off' and speeds == qsfp_fixed

    step("Step 18- clear the OVSDB config on interfaces 21, 53, and 50")
    sw_clear_user_config(ops1, test_intf)
    sw_set_intf_pm_info(ops1, test_intf,
                        ('connector=absent', 'connector_status=unsupported'))

    sw_clear_user_config(ops1, qsfp_intf)
    sw_set_intf_pm_info(ops1, qsfp_intf,
                        ('connector=absent', 'connector_status=unsupported'))

    sw_clear_user_config(ops1, split_parent)
    sw_set_intf_pm_info(ops1, split_parent,
                        ('connector=absent', 'connector_status=unsupported'))
