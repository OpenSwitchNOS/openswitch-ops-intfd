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

# from pytest import mark
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
    c = "set interface {int}".format(**locals())
    for s in conf:
        c += " user_config:{s}".format(s=s)
    return dut(c, shell="vsctl")


def sw_clear_user_config(dut, int):
    return dut("clear interface {int} user_config".format(**locals()),
               shell="vsctl")


def sw_set_intf_pm_info(dut, int, conf):
    c = "set interface {int}".format(**locals())
    for s in conf:
        c += " pm_info:{s}".format(s=s)
    return dut(c, shell="vsctl")


def sw_get_intf_state(dut, int, fields):
    c = "get interface {int}".format(**locals())
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


def test_user_config(topology, step):
    ops1 = topology.get("ops1")
    assert ops1 is not None

    ops1("/bin/systemctl stop ops-pmd", shell="bash")

    step("Step 1- Configure interface on switch s1 as no routing else the "
         "interface will use the port admin logic to set its own "
         "hw_intf_config state")
    ops1("configure terminal")
    ops1("interface {int}".format(int=test_intf))
    ops1("no routing")
    ops1("end")

    step("Step 2- Verify that interface 21 is present in the DB.")
    out = ops1("list interface {int}".format(int=test_intf), shell="vsctl")

    assert "_uuid" in out

    step("Step 3- Set admin state as 'up'")
    sw_set_intf_user_config(ops1, test_intf, ['admin=up'])
    short_sleep()

    step("Step 4- Verify that 'error' column in interface table is"
         "set to 'module_missing' due to absense of pluggable module."
         "Verify that hw_intf_config:enable is still set to 'false'.")
    err, hw_enable = sw_get_intf_state(ops1, test_intf,
                                       ['error', 'hw_intf_config:enable'])
    assert err == "module_missing" and hw_enable == '"false"'

    step("Step 5- Set admin state as 'down'")
    sw_set_intf_user_config(ops1, test_intf, ['admin=down'])
    short_sleep()

    step("Step 6- Verify that 'error' column in interface table is"
         "set to 'admin_down' as interface is disabled by user."
         "Verify that hw_intf_config:enable is still set to 'false'.")
    err, hw_enable = sw_get_intf_state(ops1, test_intf,
                                       ['error', 'hw_intf_config:enable'])
    assert err == 'admin_down' and hw_enable == '"false"'

    step("Step 7- Set the complete user_config, but leave the admin=down,"
         "verify that still interface hardware_intf_config:enable=false")
    sw_set_intf_user_config(ops1, test_intf, ['admin=down', 'autoneg=on',
                                              'speeds=1000', 'duplex=full',
                                              'pause=rxtx', 'mtu=1500'])
    short_sleep()

    step("Step 8- Verify that 'error' column in interface table is"
         "set to 'admin_down' as port is disabled by user."
         "Verify that hw_intf_config:enable is still set to 'false'.")
    err, hw_enable = sw_get_intf_state(ops1, test_intf,
                                       ['error', 'hw_intf_config:enable'])
    assert err == 'admin_down' and hw_enable == '"false"'

    step("Step 9- Clear the user config")
    sw_clear_user_config(ops1, test_intf)
    short_sleep()

    step("Step 10- Verify that interface is still disabled.")
    err, hw_enable = sw_get_intf_state(ops1, test_intf,
                                       ['error', 'hw_intf_config:enable'])
    assert err == 'admin_down' and hw_enable == '"false"'

    step("Step 11- Clear MTU and verify it is set to default.")
    sw_set_intf_pm_info(ops1, test_intf, ('connector=SFP_RJ45',
                                          'connector_status=supported'))
    sw_set_intf_user_config(ops1, test_intf, ['admin=up'])
    short_sleep()
    mtu, hw_enable = sw_get_intf_state(ops1, test_intf,
                                       ['hw_intf_config:mtu',
                                        'hw_intf_config:enable'])
    assert mtu == '"1500"' and hw_enable == '"true"'

    step("Step 12- Set MTU to illegal value (not a valid number)")
    sw_set_intf_user_config(ops1, test_intf, ['admin=up', 'mtu=1500a'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, test_intf, ['hw_intf_config:enable'])
    assert hw_enable == '"false"'

    error = sw_get_intf_state(ops1, test_intf, ['error'])
    assert error == 'invalid_mtu'

    step("Step 13- Set MTU to minimum allowed value.")
    sw_set_intf_user_config(ops1, test_intf, ['admin=up', 'mtu=576'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, test_intf, ['hw_intf_config:enable'])
    assert hw_enable == '"true"'

    mtu = sw_get_intf_state(ops1, test_intf, ['hw_intf_config:mtu'])
    assert mtu == '"576"'

    step("Step 14- Set MTU to max allowed value.")
    sw_set_intf_user_config(ops1, test_intf, ['admin=up', 'mtu=1500'])
    short_sleep()

    mtu, hw_enable = sw_get_intf_state(ops1, test_intf,
                                       ['hw_intf_config:mtu',
                                        'hw_intf_config:enable'])
    assert mtu == '"1500"' and hw_enable == '"true"'

    step("Step 15- Set MTU to value in the allowed range.")
    sw_set_intf_user_config(ops1, test_intf, ['admin=up', 'mtu=1000'])
    short_sleep()

    mtu, hw_enable = sw_get_intf_state(ops1, test_intf,
                                       ['hw_intf_config:mtu',
                                        'hw_intf_config:enable'])
    assert mtu == '"1000"' and hw_enable == '"true"'

    step("Step 16- Set MTU above allowed range.")
    sw_set_intf_user_config(ops1, test_intf, ['mtu=2000'])
    short_sleep()

    error = sw_get_intf_state(ops1, test_intf, ['error'])
    assert error == 'invalid_mtu'

    mtu = sw_get_intf_state(ops1, test_intf, ['hw_intf_config:mtu'])
    assert mtu != '"2000"'

    step("Step 17- Set MTU below allowed range.")
    sw_set_intf_user_config(ops1, test_intf, ['mtu=100'])
    short_sleep()

    error = sw_get_intf_state(ops1, test_intf, ['error'])
    assert error == 'invalid_mtu'

    mtu = sw_get_intf_state(ops1, test_intf, ['hw_intf_config:mtu'])
    assert mtu != '"100"'

    step("Step 18- Clear the user_config")
    sw_clear_user_config(ops1, test_intf)
    short_sleep()

    step("Step 19- Get the supported speeds from hw_intf_info:speeds")
    hw_info_speeds = sw_get_intf_state(ops1, test_intf,
                                       ['hw_intf_info:speeds'])
    sw_set_intf_user_config(ops1, test_intf, ['admin=up'])
    short_sleep()

    hw_enable = sw_get_intf_state(ops1, test_intf,
                                  ['hw_intf_config:enable'])
    assert hw_enable == '"true"'

    assert hw_info_speeds == '"1000,10000"'

    step("Step 20- Set user_config:speeds to a valid value(s)")
    sw_set_intf_user_config(ops1, test_intf, ['admin=up',
                                              'speeds=1000,10000'])
    short_sleep()

    speeds = sw_get_intf_state(ops1, test_intf, ['hw_intf_config:speeds'])
    assert speeds == '"1000,10000"'

    step("Step 21- Set user_config:speeds to an invalid value")
    sw_set_intf_user_config(ops1, test_intf, ['speeds=1100,10000'])
    short_sleep()

    error = sw_get_intf_state(ops1, test_intf, ['error'])
    assert error == 'invalid_speeds'

    speeds = sw_get_intf_state(ops1, test_intf, ['hw_intf_config:speeds'])
    assert speeds != '"1100,10000"'

    step("Step 22- Clear the user_config")
    sw_clear_user_config(ops1, test_intf)
    short_sleep()
