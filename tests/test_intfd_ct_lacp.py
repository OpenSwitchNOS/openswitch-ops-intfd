#!/usr/bin/python

# (c) Copyright 2016 Hewlett Packard Enterprise Development LP
#
# GNU Zebra is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
#
# GNU Zebra is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Zebra; see the file COPYING.  If not, write to the Free
# Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

from opsvsi.docker import *
from opsvsi.opsvsitest import *


class LACPCliTest(OpsVsiTest):

    def setupNet(self):

        # if you override this function, make sure to
        # either pass getNodeOpts() into hopts/sopts of the topology that
        # you build or into addHost/addSwitch calls

        host_opts = self.getHostOpts()
        switch_opts = self.getSwitchOpts()
        infra_topo = SingleSwitchTopo(k=0, hopts=host_opts, sopts=switch_opts)
        self.net = Mininet(infra_topo, switch=VsiOpenSwitch,
                           host=Host, link=OpsVsiLink,
                           controller=None, build=True)

    def addInterfacesToLags(self):
        info("########## Add interfaces to LAG ports ##########\n")
        interface_found_in_lag = False
        s1 = self.net.switches[0]
        s1.cmdCLI('conf t')
        s1.cmdCLI('interface lag 1')
        s1.cmdCLI('interface 1')
        s1.cmdCLI('lag 1')
        s1.cmdCLI('interface 2')
        s1.cmdCLI('lag 1')
        out = s1.cmdCLI('do show lacp aggregates')
        lines = out.split('\n')
        for line in lines:
            if 'Aggregated-interfaces' in line and '1' in line and '2' in line:
                interface_found_in_lag = True

        assert (interface_found_in_lag is True), \
            'Test to add interfaces to LAG ports - FAILED!'
        return True

    def showInterfaceLag(self):
        info("########## Test show interface lag command ##########\n")
        s1 = self.net.switches[0]

        # Verify 'show interface lag1' shows correct  information about lag1
        info("  ######## Verify show interface lag1 ########\n")
        success = 0
        out = s1.cmdCLI('do show interface lag1')
        lines = out.split('\n')
        for line in lines:
            if 'Aggregate-name lag1 ' in line:
                success += 1
            if 'Aggregated-interfaces' in line and '1' in line and '2' in line:
                success += 1
            if 'Speed' in line in line:
                success += 1
            if 'Aggregation-key' in line and '1' in line:
                success += 1
        assert success == 4,\
            'Test show interface lag1 command - FAILED!'

        # Verify 'show interface lag 1' shows correct error
        info("  ######## Verify show interface lag 1 ########\n")
        success = 0
        out = s1.cmdCLI('do show interface lag 1')
        lines = out.split('\n')
        for line in lines:
            if 'Unknown command' in line:
                success += 1
        assert success == 1,\
            'Test show interface lag 1 command - FAILED!'

        return True
    def show_running_config_lag_interface(self):
        info("########## Test show running-config interface lag command ##########\n")
        s1 = self.net.switches[0]

        s1.cmdCLI('configure terminal')
        s1.cmdCLI('interface lag 1')
        s1.cmdCLI('ip address 10.1.1.1/24')
        s1.cmdCLI('ipv6 address 2001::1/12')
        s1.cmdCLI('exit')
        s1.cmdCLI('exit')

        # Get information from show running-config interface
        out = s1.cmdCLI('show running-config interface')
        lines = out.split('\n')
        success = 0
        for line in lines:
            if 'ip address 10.1.1.1/24' in line:
                success += 1
            if 'ipv6 address 2001::1/12' in line:
                success += 1
        assert success == 2,\
            'Test show running-config interface command - FAILED!'

        out = s1.cmdCLI('show running-config interface lag1')
        lines = out.split('\n')
        success = 0
        for line in lines:
            if 'ip address 10.1.1.1/24' in line:
                success += 1
            if 'pv6 address 2001::1/12' in line:
                success += 1
        assert success == 2,\
            'Test show running-config interface <lag_id> - FAILED!'

        return True

    def InterfaceLagx(self):
        info("########## Test interface lagXX command ##########\n")
        s1 = self.net.switches[0]

        # Verify 'show interface lag 1' shows correct error
        info("  ######## Verify interface lag1 ########\n")
        success = 0
        out = s1.cmdCLI('interface lag1')
        lines = out.split('\n')
        for line in lines:
            if 'Unknown command' in line:
                success += 1
        assert success == 1,\
            'Test interface lag1 command - FAILED!'

        return True


@pytest.mark.skipif(True, reason="Disabling old tests")
class Test_lacp_cli:

    def setup(self):
        pass

    def teardown(self):
        pass

    def setup_class(cls):
        Test_lacp_cli.test = LACPCliTest()

    def test_addInterfacesToLags(self):
        if self.test.addInterfacesToLags():
            info('''
########## Add interfaces to LAG ports - SUCCESS! ##########
''')

    def test_showInterfaceLag(self):
        if self.test.showInterfaceLag():
            info('''
########## Test show interface lag command - SUCCESS! ##########
''')

    def test_showRunningConfigInterfaceLAG(self):
        if self.test.show_running_config_lag_interface():
            info('''
########## Test show running-config interface lag command - SUCCESS! ##########
''')
    def test_InterfaceLagx(self):
        if self.test.InterfaceLagx():
            info('''
########## Test interface lagXX command - SUCCESS! ##########
''')

    def teardown_class(cls):
        Test_lacp_cli.test.net.stop()

    def teardown_method(self, method):
        pass

    def __del__(self):
        del self.test
