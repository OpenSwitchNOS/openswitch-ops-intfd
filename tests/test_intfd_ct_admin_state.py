#!/usr/bin/python

# Copyright (C) 2015 Hewlett Packard Enterprise Development LP
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.

from opsvsi.docker import *
from opsvsi.opsvsitest import *

first_interface = "1"
second_interface = "2"
third_interface = "3"
fourth_interface = "4"
lag_interface = "lag1"

interface_down_string = "enable=false"
interface_up_string = "enable=true"
port_down = "enable=false"
port_up = "enable=true"

class adminstateupdateCTTest( OpsVsiTest ):

    def setupNet(self):
        self.net = Mininet(topo=SingleSwitchTopo(k=0, hopts=self.getHostOpts(),
                                                 sopts=self.getSwitchOpts()),
                           switch=VsiOpenSwitch,
                           host=OpsVsiHost,
                           link=OpsVsiLink, controller=None,
                           build=True)

    def test_port_interface_admin_state(self):
        info('\n########## Test intfd admin state changes for L3'\
             ' interfaces ##########\n');
        info('\n### Configuring the topology ###\n')
        s1 = self.net.switches[ 0 ]

        # Configure switch s1
        s1.cmdCLI("configure terminal")

        # Configure interface 1 on switch s1
        s1.cmdCLI("interface 1")
        s1.cmdCLI("ip address 10.0.10.1/24")
        s1.cmdCLI("ipv6 address 2000::1/120")
        s1.cmdCLI("exit")

        # Verify the interface is created with same name for L3 port
        info('###### Verify interface is created with'\
             ' same name for L3 port ######\n')
        cmd = "/usr/bin/ovs-vsctl get interface %s name" % first_interface
        output = s1.ovscmd(cmd)
        assert first_interface in output, 'Failed to create interface 1'
        info('### Interface 1 created successfully ###\n');

        # Verify the interface is created with same name for VLAN interface
        s1.cmdCLI("configure terminal")
        s1.cmdCLI("interface vlan 1")
        s1.cmdCLI("exit")
        info('###### Verify the port and interface is created with'\
             ' same name VLAN interface ######\n')
        cmd = "/usr/bin/ovs-vsctl get interface vlan%s name" % first_interface
        output = s1.ovscmd(cmd)
        expected_output = 'vlan' + first_interface
        assert expected_output in output, 'Failed to create interface 1'
        info('### interface 1 created successfully for VLAN interface ###\n');

        # Verify the interface is down by default
        info('##### Verify the hw_config state of port and interface is down by'\
             ' as interface is down ####\n')
        cmd = "/usr/bin/ovs-vsctl get interface %s hw_intf_config" % first_interface
        output = s1.ovscmd(cmd)
        assert interface_down_string in output, 'Incorrect interface default state'
        info('### Default hw_intf_config state of interface is down as expected ###\n');

        # Verify the interface associated with the port goes down
        # when port is disabled for L3 interface
        info('##### Verify the interface associated with the port'\
             ' goes down when port is disabled for L3 interface #####\n')
        s1.cmdCLI("configure terminal")
        s1.cmdCLI("interface 1")
        s1.cmdCLI("no shutdown")
        s1.cmdCLI("exit")
        cmd = "/usr/bin/ovs-vsctl get interface %s hw_intf_config" % first_interface
        output = s1.ovscmd(cmd)
        assert interface_up_string in output, 'Interface state does not change'
        # Change the admin state of port to down
        cmd = "/usr/bin/ovs-vsctl set port %s admin=down" % first_interface
        s1.ovscmd(cmd)
        # Verify the interface associated to the port is down
        cmd = "/usr/bin/ovs-vsctl get interface %s hw_intf_config" % first_interface
        output = s1.ovscmd(cmd)
        assert interface_down_string in output, 'Interface state does not change'
        info('### Interface state changed to down as the port is down ###\n');
        cmd = "/usr/bin/ovs-vsctl set port %s admin=up" % first_interface

        # Verify the interface associated with the port goes down
        # when port is disabled for VLAN interface
        info('##### Verify the interface associated with the port'\
             ' goes down when port is disabled for VLAN interface#####\n')
        s1.cmdCLI("configure terminal")
        s1.cmdCLI("interface vlan 1")
        s1.cmdCLI("no shutdown")
        s1.cmdCLI("exit")
        cmd = "/usr/bin/ovs-vsctl get interface vlan%s hw_intf_config" % first_interface
        output = s1.ovscmd(cmd)
        assert interface_up_string in output, 'Interface state does not change'
        # Change the admin state of port to down
        cmd = "/usr/bin/ovs-vsctl set port vlan%s admin=down" % first_interface
        s1.ovscmd(cmd)
        # Verify the interface associated to the port is down
        cmd = "/usr/bin/ovs-vsctl get interface vlan%s hw_intf_config" % first_interface
        output = s1.ovscmd(cmd)
        assert interface_down_string in output, 'Interface state does not change'
        info('### Interface state changed to down as the port is down ###\n');
        cmd = "/usr/bin/ovs-vsctl set port vlan%s admin=up" % first_interface

        # Verify multiple interfaces associated with LAG port goes down when LAG port is disabled
        info('Verify multiple interfaces associated with LAG port'\
             ' goes down when LAG port is disabled\n')
        s1.cmdCLI("configure terminal")
        s1.cmdCLI("interface lag 1")
        s1.cmdCLI("exit")
        cmd = "interface %s" % second_interface
        s1.cmdCLI(cmd)
        s1.cmdCLI("no shutdown")
        s1.cmdCLI("lag 1")
        s1.cmdCLI("exit")
        cmd = "interface %s" % third_interface
        s1.cmdCLI(cmd)
        s1.cmdCLI("no shutdown")
        s1.cmdCLI("lag 1")
        s1.cmdCLI("exit")
        cmd = "interface %s" % fourth_interface
        s1.cmdCLI(cmd)
        s1.cmdCLI("no shutdown")
        s1.cmdCLI("lag 1")
        s1.cmdCLI("exit")
        s1.cmdCLI("exit")
        cmd = "/usr/bin/ovs-vsctl set port %s admin=down" % lag_interface
        s1.ovscmd(cmd)
        cmd = "/usr/bin/ovs-vsctl get interface %s hw_intf_config" % second_interface
        output = s1.ovscmd(cmd)
        assert interface_down_string in output, 'Interface state does not change'
        cmd = "/usr/bin/ovs-vsctl get interface %s hw_intf_config" % third_interface
        output = s1.ovscmd(cmd)
        assert interface_down_string in output, 'Interface state does not change'
        cmd = "/usr/bin/ovs-vsctl get interface %s hw_intf_config" % fourth_interface
        output = s1.ovscmd(cmd)
        assert interface_down_string in output, 'Interface state does not change'
        info('#### All interfaces under the lag go down as soon as lag is down #####\n')

        cmd = "/usr/bin/ovs-vsctl set port %s admin=up" % lag_interface
        s1.ovscmd(cmd)
        cmd = "/usr/bin/ovs-vsctl get interface %s hw_intf_config" % second_interface
        output = s1.ovscmd(cmd)
        assert interface_up_string in output, 'Interface state does not change'
        cmd = "/usr/bin/ovs-vsctl get interface %s hw_intf_config" % third_interface
        output = s1.ovscmd(cmd)
        assert interface_up_string in output, 'Interface state does not change'
        cmd = "/usr/bin/ovs-vsctl get interface %s hw_intf_config" % fourth_interface
        output = s1.ovscmd(cmd)
        assert interface_up_string in output, 'Interface state does not change'
        info('#### All interfaces under the lag is up as soon as lag is up #####\n')

        s1.cmdCLI("configure terminal")
        cmd = "interface %s" % second_interface
        s1.cmdCLI(cmd)
        s1.cmdCLI("no lag 1")
        s1.cmdCLI("shutdown")
        s1.cmdCLI("exit")
        s1.cmdCLI("exit")
        cmd = "/usr/bin/ovs-vsctl get interface %s hw_intf_config" % second_interface
        output = s1.ovscmd(cmd)
        assert interface_down_string in output, 'Interface state does not change'
        info('#### Interfaces detached from lag gets back to default state down #####\n')

        s1.cmdCLI("configure terminal")
        s1.cmdCLI("no interface lag 1")
        s1.cmdCLI("interface 3")
        s1.cmdCLI("shutdown")
        s1.cmdCLI("interface 4")
        s1.cmdCLI("shutdown")
        s1.cmdCLI("exit")
        sleep(10)
        cmd = "/usr/bin/ovs-vsctl get interface %s hw_intf_config" % third_interface
        output = s1.ovscmd(cmd)
        assert interface_down_string in output, 'Interface state does not change'
        cmd = "/usr/bin/ovs-vsctl get interface %s hw_intf_config" % fourth_interface
        output = s1.ovscmd(cmd)
        assert interface_down_string in output, 'Interface state does not change'
        info('#### All interfaces under the lag go down as soon as lag is delete #####\n')

class Test_portd_admin_state_update:

    def setup_class(cls):
        Test_portd_admin_state_update.test = adminstateupdateCTTest()

    def teardown_class(cls):
        # Stop the Docker containers, and
        # mininet topology
        Test_portd_admin_state_update.test.net.stop()

    def test_port_interface_admin_state(self):
        self.test.test_port_interface_admin_state()

    def __del__(self):
        del self.test
