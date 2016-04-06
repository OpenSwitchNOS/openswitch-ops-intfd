#!/usr/bin/python
#
# (c) Copyright 2015 Hewlett Packard Enterprise Development LP
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

import os
import sys
import time
import subprocess
import pytest

from opsvsi.docker import *
from opsvsi.opsvsitest import *

OVS_VSCTL = "/usr/bin/ovs-vsctl "

# In X86 config Interface 50 is Splittable port.
fixed_intf = "1"
test_intf = "21"
split_parent = '50'
split_children = ['50-1', '50-2', '50-3', '50-4']
qsfp_intf = "53"

def short_sleep(tm=.5):
    time.sleep(tm)


# Set user_config for a Interface.
def sw_set_intf_user_config(sw, interface, config):
    c = "/usr/bin/ovs-vsctl set interface " + str(interface)
    for s in config:
        c += " user_config:" + s
    debug(c)
    return sw.cmd(c)


# Clear user_config for a Interface.
def sw_clear_user_config(sw, interface):
    c = "/usr/bin/ovs-vsctl clear interface " + str(interface) + " user_config"
    debug(c)
    return sw.cmd(c)


# Set pm_info for a Interface.
def sw_set_intf_pm_info(sw, interface, config):
    c = "/usr/bin/ovs-vsctl set interface " + str(interface)
    for s in config:
        c += " pm_info:" + s
    debug(c)
    return sw.cmd(c)


# Get the values of a set of columns from Interface table.
# This function returns a list of values if 2 or more
# fields are requested, and returns a single value (no list)
# if only 1 field is requested.
def sw_get_intf_state(sw, interface, fields):
    c = "/usr/bin/ovs-vsctl get interface " + str(interface)
    for f in fields:
        c += " " + f
    out = sw.ovscmd(c).splitlines()
    # If a single column value is requested,
    # then return a singleton value instead of list.
    if len(out) == 1:
        out = out[0]
    debug(out)
    return out


class intfdTest(OpsVsiTest):

    def setupNet(self):

        # Create a topology with single OpenSwitch switch and
        # one regular host.
        host_opts = self.getHostOpts()
        switch_opts = self.getSwitchOpts()
        intfd_topo = SingleSwitchTopo(k=1, hopts=host_opts, sopts=switch_opts)
        self.net = Mininet(intfd_topo, switch=VsiOpenSwitch,
                           host=Host, link=OpsVsiLink,
                           controller=None, build=True)
        self.switch1 = self.net.switches[0]


    def user_config(self):

        info("\n============= intfd user config tests =============\n")
        s1 = self.net.switches[0]

        # Configure interface on switch s1 as no routing else the interface
        # will use the port admin logic to set its own hw_intf_config state
        s1.cmdCLI("configure terminal")
        s1.cmdCLI("interface " + test_intf)
        s1.cmdCLI("no routing")
        s1.cmdCLI("exit")

        info("Verify that interface " + test_intf + " is present in the DB.\n")
        out = s1.cmd("/usr/bin/ovs-vsctl list interface " + test_intf )
        if '_uuid' not in out:
            info(out)
            assert 0, "Unable to find Interface " + test_intf + " in OVSBD.\n"

        # Set admin state as 'up'.
        sw_set_intf_user_config(s1, test_intf, ['admin=up'])
        short_sleep()

        # Verify that 'error' column in interface table is
        # set to 'module_missing' due to absense of pluggable module.
        # Verify that hw_intf_config:enable is still set to 'false'.
        info("Verify that 'error' is set to 'module_missing'\n")
        err, hw_enable = sw_get_intf_state(s1, test_intf, ['error', 'hw_intf_config:enable'])
        if err != 'module_missing' or hw_enable != 'false':
            assert 0, "Expected the interface " \
                      "'error' status to be 'module_missing', and " \
                      "'hw_intf_config:enable' status to be 'false', " \
                      "but they are error = %s, hw_intf_config:enable = %s." \
                      % (err, hw_enable)

        # Set admin state as 'down'.
        sw_set_intf_user_config(s1, test_intf, ['admin=down'])
        short_sleep()

        # Verify that 'error' column in interface table is
        # set to 'admin_down' as interface is disabled by user.
        # Verify that hw_intf_config:enable is still set to 'false'.
        info("Verify that 'error' is set to 'admin_down' when " \
             "user_config:admin=down without other params set\n")
        err, hw_enable = sw_get_intf_state(s1, test_intf, ['error', 'hw_intf_config:enable'])
        if err != 'admin_down' or hw_enable != 'false':
            assert 0, "Expected the interface " \
                      "'error' status to be 'admin_down', " \
                      "'hw_intf_config:enable' status to be 'false', " \
                      "but they are error = %s, hw_intf_config:enable = %s." \
                      % (err, hw_enable)

        # Set the complete user_config, but leave the admin=down,
        # verify that still interface hardware_intf_config:enable=false
        sw_set_intf_user_config(s1, test_intf, ['admin=down', 'autoneg=on', \
                                                'speeds=1000', 'duplex=full', \
                                                'pause=rxtx', 'mtu=1500' ])
        short_sleep()

        # Verify that 'error' column in interface table is
        # set to 'admin_down' as port is disabled by user.
        # Verify that hw_intf_config:enable is still set to 'false'.
        info("Verify that 'error' is set to 'admin_down' when admin=down " \
             "and other params are set to valid values.\n")
        err, hw_enable = sw_get_intf_state(s1, test_intf, ['error', 'hw_intf_config:enable'])
        if err != 'admin_down' or hw_enable != 'false':
            assert 0, "Expected the interface " \
                      "'error' status to be 'admin_down', " \
                      "'hw_intf_config:enable' status to be 'false', " \
                      "but they are error = %s, hw_intf_config:enable = %s." \
                      % (err, hw_enable)

        # Clear the user_config
        sw_clear_user_config(s1, test_intf)
        short_sleep()

        # Verify that interface is still disabled.
        err, hw_enable = sw_get_intf_state(s1, test_intf, ['error', 'hw_intf_config:enable'])
        if err != 'admin_down' or hw_enable != 'false':
            assert 0, "Expected the interface " \
                      "'error' status to be 'admin_down', " \
                      "'hw_intf_config:enable' status to be 'false', " \
                      "but they are error = %s, hw_intf_config:enable = %s." \
                      % (err, hw_enable)

        # MTU tests

        # Max MTU in h/w desc file (VSI) is 1500.
        # Min allowed MTU to be set is 576
        # Default MTU is 1500.

        # Clear MTU and verify it is set to default.
        sw_set_intf_pm_info(s1, test_intf, ('connector=SFP_RJ45', \
                                            'connector_status=supported'))
        sw_set_intf_user_config(s1, test_intf, ['admin=up'])
        short_sleep()

        info("Verify that MTU is set to default=1500\n")
        mtu, hw_enable = sw_get_intf_state(s1, test_intf, \
                                        ['hw_intf_config:mtu', \
                                         'hw_intf_config:enable'])
        assert mtu == '1500' and hw_enable=="true", \
                    "MTU is '%s'. Should be default=1500;" + \
                    " hw_enable is %s. should be 'true'" % (mtu, hw_enable)

        # Set MTU to illegal value (not a valid number)
        sw_set_intf_user_config(s1, test_intf, [ 'admin=up', 'mtu=1500a' ])
        short_sleep()
        info("Verify that MTU is not set to illegal value=1500a\n")

        hw_enable = sw_get_intf_state(s1, test_intf, \
                                 ['hw_intf_config:enable'])
        assert hw_enable=="false", \
                    " hw_enable is %s. should be 'false'" % hw_enable
        error = sw_get_intf_state(s1, test_intf, ['error'])
        assert error == 'invalid_mtu', \
                    "error is '%s'. Should be invalid_mtu;" % error

        # Set MTU to minimum allowed value.
        sw_set_intf_user_config(s1, test_intf, [ 'admin=up', 'mtu=576' ])
        short_sleep()
        info("Verify that MTU is set to min allowed value=576\n")

        hw_enable = sw_get_intf_state(s1, test_intf, \
                                 ['hw_intf_config:enable'])
        assert hw_enable=="true", \
                    " hw_enable is %s. should be 'true'" % hw_enable
        mtu = sw_get_intf_state(s1, test_intf, \
                                ['hw_intf_config:mtu'])
        assert mtu == '576', \
                    "MTU is '%s'. Should be 576;" % mtu

        # Set MTU to max allowed value.
        sw_set_intf_user_config(s1, test_intf, [ 'admin=up', 'mtu=1500' ])
        short_sleep()
        info("Verify that MTU is set to max allowed value=1500\n")
        mtu, hw_enable = sw_get_intf_state(s1, test_intf, \
                                ['hw_intf_config:mtu', \
                                 'hw_intf_config:enable'])
        assert mtu == '1500' and hw_enable=="true", \
                    "MTU is '%s'. Should be 1500;" + \
                    " hw_enable is %s. should be 'true'" % (mtu, hw_enable)

        # Set MTU to value in the allowed range.
        sw_set_intf_user_config(s1, test_intf, [ 'admin=up', 'mtu=1000' ])
        short_sleep()
        info("Verify that MTU is set to allowed value=1000\n")
        mtu, hw_enable = sw_get_intf_state(s1, test_intf, \
                                ['hw_intf_config:mtu', \
                                 'hw_intf_config:enable'])
        assert mtu == '1000' and hw_enable=="true", \
                    "MTU is '%s'. Should be 1000;" + \
                    " hw_enable is %s. should be 'true'" % (mtu, hw_enable)

        # Set MTU above allowed range.
        sw_set_intf_user_config(s1, test_intf, [ 'mtu=2000' ])
        short_sleep()

        info("Verify that MTU is not set when above allowed range\n")
        error = sw_get_intf_state(s1, test_intf, ['error'])
        assert error == 'invalid_mtu', \
                        "error is %s, should be 'invalid_mtu'" % error

        mtu = sw_get_intf_state(s1, test_intf, ['hw_intf_config:mtu'])
        assert mtu != '2000', "MTU incorrectly set above allowed range"

        # Set MTU below allowed range.
        sw_set_intf_user_config(s1, test_intf, [ 'mtu=100' ])
        short_sleep()

        info("Verify that MTU is not set when below allowed range\n")
        error = sw_get_intf_state(s1, test_intf, ['error'])
        assert error == 'invalid_mtu', \
                        "error is %s, should be 'invalid_mtu'" % error

        mtu = sw_get_intf_state(s1, test_intf, ['hw_intf_config:mtu'])
        assert mtu != '100', "MTU incorrectly set below allowed range"


        # "speeds" tests

        # "speeds" must be proper subset of supported speeds in hw_info

        # Clear the user_config
        sw_clear_user_config(s1, test_intf)
        short_sleep()

        # Get the supported speeds from hw_intf_info:speeds
        hw_info_speeds = sw_get_intf_state(s1, test_intf, \
                                            ['hw_intf_info:speeds'])
        sw_set_intf_user_config(s1, test_intf, [ 'admin=up' ])
        short_sleep()
        hw_enable = sw_get_intf_state(s1, test_intf, \
                                 ['hw_intf_config:enable'])
        assert hw_enable == "true", \
                    " hw_enable is %s. should be 'true'" % hw_enable

        # The expected values for hw_info_speeds is [1000,10000]

        assert hw_info_speeds == '1000,10000', \
            "unexpected values for hw_intf_info:speeds, expected " + \
            "1000,10000 but got %s" % hw_info_speeds

        # Set user_config:speeds to a valid value(s)
        sw_set_intf_user_config(s1, test_intf, [ 'admin=up', \
                                            'speeds=1000,10000' ])
        short_sleep()
        info("Verify valid speeds accepted\n")
        speeds = sw_get_intf_state(s1, test_intf, ['hw_intf_config:speeds'])
        assert speeds == '1000,10000', "speeds is %s. Should be '1000,10000'" \
                                                % (speeds)

        # Set user_config:speeds to an invalid value
        sw_set_intf_user_config(s1, test_intf, [ 'speeds=1100,10000' ])
        short_sleep()

        info("Verify invalid speeds rejected\n")
        error = sw_get_intf_state(s1, test_intf, ['error'])
        assert error == 'invalid_speeds', \
               "error is %s, should be 'invalid_speeds'" % error

        speeds = sw_get_intf_state(s1, test_intf, ['hw_intf_config:speeds'])
        assert speeds != '1100,10000', "invalid speeds accepted!"

        # Clear the user_config
        sw_clear_user_config(s1, test_intf)
        short_sleep()

    # Set pm_info to valid values, and verify that 'intfd'
    # changes the "error" state to something other than 'module_missing'
    def pm_info_detect(self):

        info("\n============= intfd pm_info detect tests =============\n")
        s1 = self.net.switches[0]

        # Enable the interface, and set the pm_info to valid values.
        sw_clear_user_config(s1, test_intf)
        sw_set_intf_user_config(s1, test_intf, ['admin=up'])
        sw_set_intf_pm_info(s1, test_intf, ('connector=SFP_RJ45', 'connector_status=supported'))
        short_sleep()

        # Make sure that 'error' status is not 'module_missing'
        info("Verify that interface pluggable module status is correctly detected.\n")
        err = sw_get_intf_state(s1, test_intf, ['error'])
        if 'module_missing' == err:
            assert 0, "Expected the interface error column value is not equal to 'module_missing'"

        # Set pm_info:connector as 'absent'
        sw_set_intf_pm_info(s1, test_intf, ('connector=absent', 'connector_status=unrecognized'))
        short_sleep()

        # Make sure that 'error' status is set to 'module_missing'
        info("Verify that pluggable module removal is correctly detected.\n")
        err, hw_enable = sw_get_intf_state(s1, test_intf, ['error', 'hw_intf_config:enable'])
        if err != 'module_missing' or hw_enable != 'false':
            assert 0, "Expected the interface " \
                      "'error' status to be 'module_missing', " \
                      "'hw_intf_config:enable' status to be 'false', " \
                      "but they are error = %s, hw_intf_config:enable = %s." \
                      % (err, hw_enable)

        # Set pm_info:connector as 'unknown' and connector_status=unrecognized
        sw_set_intf_pm_info(s1, test_intf, ('connector=unknown', 'connector_status=unrecognized'))
        short_sleep()

        info("Verify that when connector_status=unrecognized, error is module_unrecognized.\n")
        err, hw_enable = sw_get_intf_state(s1, test_intf, ['error', 'hw_intf_config:enable'])
        assert err == 'module_unrecognized' and hw_enable == 'false', \
               "Invalid interface status when unrecognized pluggable module is inserted"

        # Set pm_info:connector as 'unknown' and connector_status=unsupported
        sw_set_intf_pm_info(s1, test_intf, ('connector=unknown', 'connector_status=unsupported'))
        short_sleep()

        info("Verify that when connector_status=unsupported, error is module_unsupported.\n")
        err, hw_enable = sw_get_intf_state(s1, test_intf, ['error', 'hw_intf_config:enable'])
        assert err == 'module_unsupported' and hw_enable == 'false', \
               "Invalid interface status when unsupported pluggable module is inserted"

        # Set pm_info:connector as <unknown to intfd, such as SFP_LX>  and
        #    connector_status=supported
        sw_set_intf_pm_info(s1, test_intf, ('connector=SFP_LX', 'connector_status=supported'))
        short_sleep()

        info("Verify that when connector is unknown to intfd but pmd says supported, error is module_unsupported.\n")
        err, hw_enable = sw_get_intf_state(s1, test_intf, ['error', 'hw_intf_config:enable'])
        assert err == 'module_unsupported' and hw_enable == 'false', \
               "Invalid interface status when intfd unsupported pluggable module is inserted"

        # Verify the hw_intf_config:interface_type based on 'pm_info:connector'
        pm_intf_type = {
            'SFP_RJ45' : '1GBASE_T',
            'SFP_SX' : '1GBASE_SX',
            'SFP_SR' : '10GBASE_SR',
            'SFP_LR' : '10GBASE_LR',
            'SFP_DAC' : '10GBASE_CR' }

        for pm_type,intf_type in pm_intf_type.items():
            sw_set_intf_pm_info(s1, test_intf, ['connector=' + pm_type, 'connector_status=supported'])
            short_sleep()

            info("Verify that 'hw_intf_config:interface_type' is set to " + intf_type + \
                 " when connector=" + pm_type + "\n")
            out = sw_get_intf_state(s1, test_intf, ['hw_intf_config:interface_type'])
            assert intf_type == out, "hw_intf_type configuration in hw_intf_config is wrong."

        # Configure interface on switch s1 as no routing else the interface
        # will use the port admin logic to set its own hw_intf_config state
        s1.cmdCLI("configure terminal")
        s1.cmdCLI("interface " + split_parent)
        s1.cmdCLI("no routing")
        s1.cmdCLI("exit")
        # In X86 config Interface 50 is Splittable port.
        # Enable the split parent interface.
        sw_set_intf_user_config(s1, split_parent, ['admin=up', 'lane_split=no-split'])

        pm_intf_type = {
            'SFP_SR' : '10GBASE_SR',
            'SFP_LR' : '10GBASE_LR',
            'SFP_DAC' : '10GBASE_CR',
            'QSFP_CR4' : '40GBASE_CR4',
            'QSFP_SR4' : '40GBASE_SR4',
            'QSFP_LR4' : '40GBASE_LR4' }

        for pm_type, intf_type in pm_intf_type.items():
            sw_set_intf_pm_info(s1, split_parent, ['connector=' + pm_type, 'connector_status=supported'])
            short_sleep()

            info("Verify that 'hw_intf_config:interface_type' is set to " + intf_type + \
                 " when connector=" + pm_type + " on QSFP ports.\n")
            out = sw_get_intf_state(s1, split_parent, ['hw_intf_config:interface_type'])
            assert intf_type == out, "hw_intf_type configuration in hw_intf_config is wrong for split parent."

        # Enable the split children port.
        sw_set_intf_user_config(s1, split_parent, ['admin=down', 'lane_split=split'])
        for child_port in split_children:
            # Configure interface on switch s1 as no routing else the interface
            # will use the port admin logic to set its own hw_intf_config state
            s1.cmdCLI("configure terminal")
            s1.cmdCLI("interface " + child_port)
            s1.cmdCLI("no routing")
            s1.cmdCLI("exit")
            sw_set_intf_user_config(s1, child_port, ['admin=up']);

        # HW intf config for split children.
        pm_intf_type = {
            'QSFP_CR4' : '10GBASE_CR',
            'QSFP_SR4' : '10GBASE_SR',
            'QSFP_LR4' : '10GBASE_LR' }

        for pm_type, intf_type in pm_intf_type.items():
            sw_set_intf_pm_info(s1, split_parent, ['connector=' + pm_type, 'connector_status=supported'])
            short_sleep()

            info("Verify that 'hw_intf_config:interface_type' is set to " + intf_type + \
                 " when connector=" + pm_type + " on QSFP split ports.\n")
            for child_port in split_children:
                out = sw_get_intf_state(s1, child_port, ['hw_intf_config:interface_type'])
                assert intf_type == out, "hw_intf_type configuration in hw_intf_config is wrong for split child."


        # Unsupported pluggable modules when interface is split
        pm_intf_type = [ 'SFP_RJ45', 'SFP_SX', 'SFP_SR', 'SFP_LR', 'SFP_DAC' ]

        for pm_type in pm_intf_type:
            sw_set_intf_pm_info(s1, split_parent, ['connector=' + pm_type, 'connector_status=supported'])
            short_sleep()

            info("Verify that split port is disabled when unsupported module %s is inserted.\n" % pm_type)
            for child_port in split_children:
                err, hw_enable = sw_get_intf_state(s1, child_port, ['error', 'hw_intf_config:enable'])
                assert err == 'module_unsupported' and hw_enable == 'false', \
                       "Split children should not be enabled when non QSFP module is inserted."

        # Clear the OVSDB config of the interfaces
        sw_clear_user_config(s1, test_intf)
        sw_set_intf_pm_info(s1, test_intf, ('connector=absent', 'connector_status=unsupported'))

        sw_clear_user_config(s1, split_parent);
        sw_set_intf_pm_info(s1, split_parent, ('connector=absent', 'connector_status=unsupported'))

        for child_port in split_children:
            sw_clear_user_config(s1, child_port);


    def user_config_autoneg(self):

        info("\n============= intfd user config autoneg tests =============\n")
        s1 = self.net.switches[0]
        sw_clear_user_config(s1, test_intf);
        sw_clear_user_config(s1, split_parent);
        sw_set_intf_user_config(s1, test_intf, ['admin=up'])
        sw_set_intf_user_config(s1, split_parent, ['admin=up', 'lane_split=no-split'])

        pm_autoneg = {
            'SFP_RJ45' : 'on',
            'SFP_SX' : 'on',
            'SFP_SR' : 'off',
            'SFP_LR' : 'off',
            'SFP_DAC' : 'off' }

        for pm_type, autoneg in pm_autoneg.items():
            sw_set_intf_pm_info(s1, test_intf, ["connector=" + pm_type, 'connector_status=supported'])
            short_sleep()

            info("Verify that 'hw_intf_config:autoneg' is set to " + autoneg + \
                 " when connector=" + pm_type + "\n")
            out = sw_get_intf_state(s1, test_intf, ['hw_intf_config:autoneg'])
            assert autoneg == out, "Autoneg configuration in hw_intf_config is wrong."

        pm_autoneg = {
            'SFP_RJ45' : 'on',
            'SFP_SR' : 'off',
            'SFP_LR' : 'off',
            'SFP_DAC' : 'off',
            'QSFP_CR4' : 'on',
            'QSFP_SR4' : 'off',
            'QSFP_LR4' : 'off' }

        for pm_type, autoneg in pm_autoneg.items():
            sw_set_intf_pm_info(s1, split_parent, ['connector=' + pm_type, 'connector_status=supported'])
            short_sleep()

            info("Verify that 'hw_intf_config:autoneg' is set to " + autoneg + \
                 " when connector=" + pm_type + " for split parent interface.\n")
            out = sw_get_intf_state(s1, split_parent, ['hw_intf_config:autoneg'])
            assert autoneg == out, "Autoneg configuration in hw_intf_config is wrong for split parent interface."

        # Enable the split children port.
        sw_set_intf_user_config(s1, split_parent, ['admin=down', 'lane_split=split'])
        for child_port in split_children:
            sw_set_intf_user_config(s1, child_port, ['admin=up']);

        # HW intf config for split children.
        pm_autoneg = {
            'QSFP_CR4' : 'off',
            'QSFP_SR4' : 'off',
            'QSFP_LR4' : 'off' }

        for pm_type, autoneg in pm_autoneg.items():
            sw_set_intf_pm_info(s1, split_parent, ['connector=' + pm_type, 'connector_status=supported'])
            short_sleep()

            info("Verify that 'hw_intf_config:autoneg' is set to " + autoneg + \
                 " when connector=" + pm_type + " on QSFP split ports.\n")
            for child_port in split_children:
                out = sw_get_intf_state(s1, child_port, ['hw_intf_config:autoneg'])
                assert autoneg == out, "Autgoneg configuration in hw_intf_config is wrong for split child."


        # Test the interaction of user input for speeds and AN for the
        # various interface types to ensure AN and speeds are properly set.

        # The following describes what is expected.
        #
        # First, specify what interface types support or require AN.
        #    SFP:        autoneg is REQUIRED, speeds = supported_speeds
        #    SFP+:       autoneg is NOT_SUPPORTED, speeds = 10G
        #    QSFP+.CR4:  autoneg is REQUIRED, speeds = 40G
        #    QSFP+.else: autoneg is NOT_SUPPORTED, speeds = 40G
        #    else:       autoneg is REQUIRED, speeds = 0
        #
        # Now indicate impact of user supplied AN/speeds on the above defaults
        #
        # supported_speeds: The list of supported speeds in hw desc file
        # fixed_speed: Set speed if AN is not supported
        # highest_speed: Highest speed in supported_speed list
        # first_speed: First speed in the list supplied by customer
        # user_speeds: Speeds list supplied by customer
        # speeds: Is set to one of the above based on transceiver type,
        #         user AN setting, and user speeds setting.
        #
        # AN not specified, user_speeds not specified:
        #   if AN supported or required: AN=true, speeds=supported_speeds
        #   if AN not_supported: AN=false, speeds=fixed_speed
        # AN=T, user_speeds not specified
        #   if AN supported or required: AN=true, speeds=supported_speeds
        #   if AN not_supported: user error! "AN not supported"
        # AN=F, user_speeds not specified
        #   if AN supported: AN=false, speeds=highest_speed
        #   if AN required: user error! "AN required"
        #   if AN not_supported: AN=false, speeds=fixed_speed
        # AN not specified, user_speeds specified:
        #   if AN supported or required: AN=true, speeds=user_speeds
        #   if AN not_supported: AN=false, speeds=first_speed
        # AN=T, user_speeds specified
        #   if AN supported or required: AN=true, speeds=user_speeds
        #   if AN not_supported: user error! "AN not supported"
        # AN=F, user_speeds specified
        #   if AN supported: AN=false, speeds=first_speed
        #   if AN required: user error! "AN required"
        #   if AN not_supported: AN=false, speeds=first_speed

        # Tests follow for the above cases
        sw_clear_user_config(s1, test_intf)
        sw_clear_user_config(s1, qsfp_intf)

        sfp_fixed = '1000'
        qsfp_fixed = '40000'

        #########   AN not specified, user_speeds not specified:  ########

        #   if AN supported or required: AN=true, user_speeds=supported_speeds
        sw_set_intf_pm_info(s1, test_intf, ('connector=SFP_RJ45', \
                                          'connector_status=supported'))
        short_sleep()
        sw_set_intf_user_config(s1, test_intf, ['admin=up'])
        short_sleep()
        info("Verify AN not specified, user_speeds not specified, " + \
                                                "AN supported/required\n");
        autoneg, speeds = sw_get_intf_state(s1, test_intf, \
                        ['hw_intf_config:autoneg', 'hw_intf_config:speeds'])

        assert autoneg == 'on' and speeds == sfp_fixed, \
            "autoneg is %s, expected 'on'; speeds is %s, expected %s" \
                                        % (autoneg, speeds, sfp_fixed)

        #   if AN not_supported: AN=false, speeds=fixed_speed
        sw_set_intf_pm_info(s1, qsfp_intf, ('connector=QSFP_SR4',  \
                                            'connector_status=supported'))
        short_sleep()
        # Configure interface on switch s1 as no routing else the interface
        # will use the port admin logic to set its own hw_intf_config state
        s1.cmdCLI("configure terminal")
        s1.cmdCLI("interface " + qsfp_intf)
        s1.cmdCLI("no routing")
        s1.cmdCLI("exit")
        sw_set_intf_user_config(s1, qsfp_intf, ['admin=up'])
        short_sleep()
        info("Verify AN not specified, user_speeds not specified, AN unsupported\n");
        hw_enable = sw_get_intf_state(s1, qsfp_intf, ['hw_intf_config:enable'])
        assert hw_enable == "true", "hw_enable should be true and is not"

        autoneg, speeds = sw_get_intf_state(s1, qsfp_intf, \
                        ['hw_intf_config:autoneg', 'hw_intf_config:speeds'])

        assert autoneg == 'off' and speeds == qsfp_fixed, \
            "autoneg is %s, expected 'off'; speeds is %s, expected %s" \
                                        % (autoneg, speeds, qsfp_fixed)

        ###########   AN=T, user_speeds not specified    ##################

        #   if AN supported or required: AN=true, speeds=supported_speeds
        sw_set_intf_pm_info(s1, test_intf, ('connector=SFP_RJ45', \
                                          'connector_status=supported'))
        short_sleep()
        sw_set_intf_user_config(s1, test_intf, ['admin=up', 'autoneg=on'])
        short_sleep()
        info("Verify AN set=true, user_speeds not specified, " + \
                                                "AN supported/required\n");
        autoneg, speeds = sw_get_intf_state(s1, test_intf, \
                        ['hw_intf_config:autoneg', 'hw_intf_config:speeds'])

        assert autoneg == 'on' and speeds == sfp_fixed, \
            "autoneg is %s, expected 'on'; speeds is %s, expected %s" \
                                        % (autoneg, speeds, sfp_fixed)

        #   if AN not_supported: user error! "AN not supported"
        sw_set_intf_pm_info(s1, qsfp_intf, ('connector=QSFP_SR4',  \
                                            'connector_status=supported'))
        short_sleep()
        sw_set_intf_user_config(s1, qsfp_intf, ['admin=up', 'autoneg=on'])
        short_sleep()
        info("Verify AN set=true, user_speeds not specified, AN unsupported\n");
        hw_enable = sw_get_intf_state(s1, qsfp_intf, ['hw_intf_config:enable'])
        assert hw_enable == "false", "hw_enable should be false and is not"
        error = sw_get_intf_state(s1, qsfp_intf, ['error'])
        assert error == "autoneg_not_supported", \
                   "error is [%s] and should be autoneg_not_supported" % error

        ###########     AN=F, user_speeds not specified    ############

        #   if AN supported: AN=false, speeds=highest_speed
        #
        #     NOTE: We don't currently have a type that is supported
        #           but not required.
        #

        #   if AN required: user error! "AN required"
        sw_set_intf_pm_info(s1, test_intf, ('connector=SFP_RJ45', \
                                          'connector_status=supported'))
        short_sleep()
        sw_set_intf_user_config(s1, test_intf, ['admin=up', 'autoneg=off'])
        short_sleep()
        info("Verify AN set=false, user_speeds not specified, " + \
                                                "AN required\n");
        hw_enable = sw_get_intf_state(s1, test_intf, ['hw_intf_config:enable'])
        assert hw_enable == "false", "hw_enable should be false and is not"
        error = sw_get_intf_state(s1, test_intf, ['error'])
        assert error == "autoneg_required", \
                      "error is [%s] and should be autoneq_required" % error

        #   if AN not_supported: AN=false, speeds=fixed_speed
        sw_set_intf_pm_info(s1, qsfp_intf, ('connector=QSFP_LR4', \
                                          'connector_status=supported'))
        short_sleep()
        sw_set_intf_user_config(s1, qsfp_intf, ['admin=up', 'autoneg=off'])
        short_sleep()
        info("Verify AN set=false, user_speeds not specified, " + \
                                                "AN unsupported\n");
        hw_enable = sw_get_intf_state(s1, qsfp_intf, ['hw_intf_config:enable'])
        assert hw_enable == "true", "hw_enable should be true and is not"

        autoneg, speeds = sw_get_intf_state(s1, qsfp_intf, \
                        ['hw_intf_config:autoneg', 'hw_intf_config:speeds'])

        assert autoneg == 'off' and speeds == qsfp_fixed, \
            "autoneg is %s, expected 'off'; speeds is %s, expected %s" \
                                        % (autoneg, speeds, qsfp_fixed)

        ###########    AN not specified, user_speeds specified:    ############

        #   if AN supported or required: AN=true, speeds=cust_speeds
        sw_clear_user_config(s1, test_intf)
        short_sleep()
        sw_set_intf_pm_info(s1, test_intf, ('connector=SFP_RJ45', \
                                          'connector_status=supported'))
        short_sleep()
        sw_set_intf_user_config(s1, test_intf, ['admin=up', 'speeds=1000'])
        short_sleep()
        info("Verify AN not specified, user_speeds specified, " + \
                                                "AN supported/required\n");
        hw_enable = sw_get_intf_state(s1, test_intf, ['hw_intf_config:enable'])
        assert hw_enable == "true", "hw_enable should be true and is not"

        autoneg, speeds = sw_get_intf_state(s1, test_intf, \
                        ['hw_intf_config:autoneg', 'hw_intf_config:speeds'])

        assert autoneg == 'on' and speeds == '1000', \
            "autoneg is %s, expected 'on'; speeds is %s, expected %s" \
                                        % (autoneg, speeds, '1000')

        #   if AN not_supported: AN=false, speeds=first_speed
        sw_clear_user_config(s1, qsfp_intf)
        short_sleep()
        sw_set_intf_pm_info(s1, qsfp_intf, ('connector=QSFP_LR4', \
                                          'connector_status=supported'))
        short_sleep()
        sw_set_intf_user_config(s1, qsfp_intf, ['admin=up', 'speeds=40000'])
        short_sleep()

        info("Verify AN not specified, user_speeds specified, AN unsupported\n");
        hw_enable = sw_get_intf_state(s1, qsfp_intf, ['hw_intf_config:enable'])
        assert hw_enable == "true", "hw_enable should be true and is not"

        autoneg, speeds = sw_get_intf_state(s1, qsfp_intf, \
                        ['hw_intf_config:autoneg', 'hw_intf_config:speeds'])

        assert autoneg == 'off' and speeds == qsfp_fixed, \
            "autoneg is %s, expected 'off'; speeds is %s, expected %s" \
                                        % (autoneg, speeds, qsfp_fixed)

        ###########    AN=T, user_speeds specified    ############

        #   if AN supported or required: AN=true, speeds=cust_speeds
        short_sleep()
        sw_set_intf_pm_info(s1, test_intf, ('connector=SFP_RJ45', \
                                          'connector_status=supported'))
        short_sleep()
        sw_set_intf_user_config(s1, test_intf, ['admin=up', 'autoneg=on', \
                                                        'speeds=1000'])
        short_sleep()
        info("Verify AN set=true, user_speeds specified, " + \
                                                "AN supported/required\n");
        hw_enable = sw_get_intf_state(s1, test_intf, ['hw_intf_config:enable'])
        assert hw_enable == "true", "hw_enable should be true and is not"

        autoneg, speeds = sw_get_intf_state(s1, test_intf, \
                        ['hw_intf_config:autoneg', 'hw_intf_config:speeds'])

        assert autoneg == 'on' and speeds == '1000', \
            "autoneg is %s, expected 'on'; speeds is %s, expected %s" \
                                        % (autoneg, speeds, '1000')

        #   if AN not_supported: user error! "AN not supported"
        short_sleep()
        sw_set_intf_pm_info(s1, qsfp_intf, ('connector=QSFP_LR4', \
                                          'connector_status=supported'))
        short_sleep()
        sw_set_intf_user_config(s1, qsfp_intf, ['admin=up', 'autoneg=on', \
                                                'speeds=40000'])
        short_sleep()

        info("Verify AN set=true, user_speeds specified, AN unsupported\n");
        hw_enable = sw_get_intf_state(s1, qsfp_intf, ['hw_intf_config:enable'])
        assert hw_enable == "false", "hw_enable should be false and is not"
        error = sw_get_intf_state(s1, qsfp_intf, ['error'])
        assert error == "autoneg_not_supported", \
                 "error is [%s] and should be autoneq_not_supported" % error

        ###########    AN=F, user_speeds specified    ############

        #   if AN supported: AN=false, speeds=first_speed
        #
        #     NOTE: We don't currently have a type that is supported
        #           but not required.
        #

        #   if AN required: user error! "AN required"
        sw_set_intf_pm_info(s1, test_intf, ('connector=SFP_RJ45', \
                                          'connector_status=supported'))
        short_sleep()
        sw_set_intf_user_config(s1, test_intf, ['admin=up', 'autoneg=off', \
                                                          'speeds=1000'])
        short_sleep()
        info("Verify AN set=false, user_speeds specified, AN required\n");
        hw_enable = sw_get_intf_state(s1, test_intf, ['hw_intf_config:enable'])
        assert hw_enable == "false", "hw_enable should be false and is not"
        error = sw_get_intf_state(s1, test_intf, ['error'])
        assert error == "autoneg_required", \
                      "error is [%s] and should be autoneq_required" % error

        #   if AN not_supported: AN=false, speeds=first_speed
        short_sleep()
        sw_set_intf_pm_info(s1, qsfp_intf, ('connector=QSFP_LR4', \
                                          'connector_status=supported'))
        short_sleep()
        sw_set_intf_user_config(s1, qsfp_intf, ['admin=up', 'autoneg=off', \
                                                        'speeds=40000'])
        short_sleep()

        info("Verify AN set=false, user_speeds specified, AN unsupported\n");
        hw_enable = sw_get_intf_state(s1, qsfp_intf, ['hw_intf_config:enable'])
        assert hw_enable == "true", "hw_enable should be true and is not"

        autoneg, speeds = sw_get_intf_state(s1, qsfp_intf, \
                        ['hw_intf_config:autoneg', 'hw_intf_config:speeds'])

        assert autoneg == 'off' and speeds == qsfp_fixed, \
            "autoneg is %s, expected 'off'; speeds is %s, expected %s" \
                                        % (autoneg, speeds, qsfp_fixed)

        # clear the OVSDB config of the interface 21
        sw_clear_user_config(s1, test_intf)
        sw_set_intf_pm_info(s1, test_intf, \
                ('connector=absent', 'connector_status=unsupported'))

        # clear the OVSDB config of the interface 53
        sw_clear_user_config(s1, qsfp_intf)
        sw_set_intf_pm_info(s1, qsfp_intf, \
                ('connector=absent', 'connector_status=unsupported'))

        # clear the OVSDB config of the interface 50
        sw_clear_user_config(s1, split_parent)
        sw_set_intf_pm_info(s1, split_parent, \
                ('connector=absent', 'connector_status=unsupported'))

    def user_config_pause(self):

        info("\n============= intfd user config pause tests =============\n")
        s1 = self.net.switches[0]

        # Set user_config & pm_info to valid values.
        sw_clear_user_config(s1, test_intf)
        sw_set_intf_user_config(s1, test_intf, ['admin=up'])
        sw_set_intf_pm_info(s1, test_intf, ('connector=SFP_RJ45', 'connector_status=supported'))
        short_sleep()

        pause_values = ('none', 'rx', 'tx', 'rxtx')

        for val in pause_values:
            info("Testing user_config:pause parameter with %s\n" % val)
            sw_set_intf_user_config(s1, test_intf, ["pause=%s" % val])
            short_sleep()

            out = sw_get_intf_state(s1, test_intf, ['hw_intf_config:pause'])
            assert val == out, "pause configuration in hw_intf_config is wrong for pause=%s\n" % val

        # Clear the OVSDB config of the interface.
        sw_clear_user_config(s1, test_intf)
        sw_set_intf_pm_info(s1, test_intf, ('connector=absent', 'connector_status=unsupported'))


    def user_config_speeds(self):
        # OPS_TODO: Fixed speed calculations based on the
        # pluggable module is not working as expected.
        # Once it is corrected, implement the tests based on that.
        pass


    def user_config_qsfp_splitter(self):

        info("\n============= intfd user config QSFP splitter tests =============\n")
        s1 = self.net.switches[0]

        sw_clear_user_config(s1, split_parent)
        # Verify that the default state of parent interface is 'admin_down'
        info("Verify that default 'error' status for split parent is 'admin_down'\n")
        out = sw_get_intf_state(s1, split_parent, ['error'])
        assert 'admin_down' == out, "Initial status of QSFP split parent interface is incorrect."

        # Verify that the default state of children ports is 'lanes_not_split'
        info("Verify that 'error' status for split children is 'lanes_not_split'\n")
        for child_port in split_children:
            out = sw_get_intf_state(s1, child_port, ['error'])
            assert 'lanes_not_split' == out, "Initial status of QSFP split children port is incorrect."

        # Set Pluggable module info to valid values on parent interface.
        sw_set_intf_pm_info(s1, split_parent, ('connector=QSFP_CR4', 'connector_status=supported'))

        # Make sure that split children are disabled as long as
        # user_config:lane_split=no-split on the parent interface.
        sw_set_intf_user_config(s1, split_parent, ['admin=up', 'lane_split=no-split'])

        info("Verify that parent interface is enabled when 'lane_split=no-split'\n");
        hw_enable = sw_get_intf_state(s1, split_parent, ['hw_intf_config:enable'])
        assert hw_enable == 'true', "Invalid parent interface status. Expected the port to be up"

        # Enable the split children via user_config, but they should be still
        # disabled in the hw_intf_config.
        for child_port in split_children:
            sw_set_intf_user_config(s1, child_port, ['admin=up'])

        short_sleep()
        info("Verify that split children ports are disabled when 'lane_split=no-split'\n");
        for child_port in split_children:
            hw_enable = sw_get_intf_state(s1, child_port, ['hw_intf_config:enable'])
            assert hw_enable == 'false', "Invalid split child port status. Expected the port to be disabled"

        # Split the parent interface and make sure that parent interface is disabled, and
        # split child ports are enabled.
        sw_set_intf_user_config(s1, split_parent, ['admin=up', 'lane_split=split'])
        short_sleep()

        info("Verify that parent interface is disabled when 'lane_split=split'\n");
        err, hw_enable = sw_get_intf_state(s1, split_parent, ['error', 'hw_intf_config:enable'])
        assert err == 'lanes_split' and hw_enable == 'false', \
               "Invalid parent interface status. Expected the port to be down"

        info("Verify that split children ports are enabled when 'lane_split=split'\n");
        for child_port in split_children:
            hw_enable = sw_get_intf_state(s1, child_port, ['hw_intf_config:enable'])
            assert hw_enable == 'true', "Invalid split child port status. Expected the port to be enabled"

        info("Verify that disabling parent interface doesn't affect split ports when 'lane_split=split'\n");
        sw_set_intf_user_config(s1, split_parent, ['admin=down'])

        info("Verify that split children ports are enabled even if parent is disabled.\n");
        for child_port in split_children:
            hw_enable = sw_get_intf_state(s1, child_port, ['hw_intf_config:enable'])
            assert hw_enable == 'true', "Invalid split child port status. Expected the port to be enabled"

        # Change the lane_split to 'no-split', then all child ports should be disabled.
        info("Verify that all child ports are disabled when lane_split on parent interface is changed to no-split.\n")
        sw_set_intf_user_config(s1, split_parent, ['lane_split=no-split'])
        for child_port in split_children:
            hw_enable = sw_get_intf_state(s1, child_port, ['hw_intf_config:enable'])
            assert hw_enable == 'false', "Invalid split child port status. Expected the port to be disabled"

        # Cleanup
        sw_clear_user_config(s1, split_parent);
        sw_set_intf_pm_info(s1, split_parent, ('connector=absent', 'connector_status=unsupported'))

        for child_port in split_children:
            sw_clear_user_config(s1, child_port);

    def fixed_1G_fixed_ports(self):

        info("\n============= intfd fixed 1G ports tests =============\n")
        s1 = self.net.switches[0]
        # Configure interface fixed_intf on switch s1 as no routing else the interface
        # will use the port admin logic to set its own hw_intf_config state
        s1.cmdCLI("configure terminal")
        s1.cmdCLI("interface " + fixed_intf)
        s1.cmdCLI("no routing")
        s1.cmdCLI("exit")

        # In VSI environment interfaces 1 - 10 are supposed to be fixed,
        # without any pluggable modules. If not skip this test.
        is_pluggable, connector = sw_get_intf_state(s1, fixed_intf, ['hw_intf_info:pluggable', \
                                                                     'hw_intf_info:connector'])
        if is_pluggable != 'false' or connector != 'RJ45':
            info("Interface " + fixed_intf + " is not fixed RJ45 connector as expected. Skipping tests.")
            return

        # The default state of these interfaces should be 'admin_down'
        err =  sw_get_intf_state(s1, fixed_intf, ['error'])
        assert err == 'admin_down', "The default state of fixed interface is wrong."

        # Enable the interface it should come up.
        sw_set_intf_user_config(s1, fixed_intf, ['admin=up'])

        hw_enable, autoneg, intf_type = sw_get_intf_state(s1, fixed_intf, \
                                                          ['hw_intf_config:enable', \
                                                           'hw_intf_config:autoneg', \
                                                           'hw_intf_config:interface_type'])
        info("Verify that fixed interfaces can come up without any pluggable info.\n")
        assert hw_enable == 'true' and autoneg == 'on' and intf_type == '1GBASE_T', \
               "Incorrect hw_intf_config state of fixed interface after enabling it."

        # Clear the user_config.
        sw_clear_user_config(s1, fixed_intf)


@pytest.mark.skipif(True, reason="Disabling old tests")
class Test_intfd:

    def setup(self):
        pass

    def teardown(self):
        pass

    def setup_class(cls):
        # Create the Mininet topology based on mininet.
        Test_intfd.test = intfdTest()

        # Stop PMD. This tests manually sets lot of DB elements
        # that 'pmd' is responsible for. To avoid any cross
        # interaction disable 'pmd'
        Test_intfd.test.switch1.cmd("/bin/systemctl stop ops-pmd")
        pass

    def teardown_class(cls):
        Test_intfd.test.switch1.cmd("/bin/systemctl start ops-pmd")
        # Stop the Docker containers, and
        # mininet topology
        Test_intfd.test.net.stop()

    def setup_method(self, method):
        pass

    def teardown_method(self, method):
        pass

    def __del__(self):
        del self.test

    # Interface daemon tests.
    def test_intfd_user_config(self):
        self.test.user_config()

    def test_intfd_pm_info(self):
        self.test.pm_info_detect()

    def test_intfd_user_config_autoneg(self):
        self.test.user_config_autoneg()

    def test_intfd_user_config_pause(self):
        self.test.user_config_pause()

    def test_intfd_user_config_speeds(self):
        self.test.user_config_speeds()

    def test_intfd_user_config_qsfp_splitter(self):
        self.test.user_config_qsfp_splitter()

    def test_intfd_1G_fixed_ports(self):
        self.test.fixed_1G_fixed_ports()
