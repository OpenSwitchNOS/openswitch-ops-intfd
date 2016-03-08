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
import re


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

    def createLagPort(self):
        info('''
########## Test to create LAG Port ##########
''')
        lag_port_found = False
        s1 = self.net.switches[0]
        s1.cmdCLI('conf t')
        s1.cmdCLI('interface lag 1')
        s1.cmdCLI('interface lag 2')
        out = s1.cmd('ovs-vsctl list port')
        lines = out.split('\n')
        for line in lines:
            if '"lag1"' in line:
                lag_port_found = True
        assert (lag_port_found is True), \
            'Test to create LAG Port - FAILED!!'
        return True

    def addInterfacesToLags(self):
        info('''
########## Test to add interfaces to LAG ports ##########
''')
        interface_found_in_lag = False
        s1 = self.net.switches[0]
        s1.cmdCLI('conf t')
        s1.cmdCLI('interface 1')
        s1.cmdCLI('lag 1')
        s1.cmdCLI('interface 2')
        s1.cmdCLI('lag 1')
        s1.cmdCLI('interface 3')
        s1.cmdCLI('lag 2')
        s1.cmdCLI('interface 4')
        s1.cmdCLI('lag 2')
        out = s1.cmdCLI('do show lacp aggregates')
        lines = out.split('\n')
        for line in lines:
            if 'Aggregated-interfaces' in line and '3' in line and '4' in line:
                interface_found_in_lag = True

        assert (interface_found_in_lag is True), \
            'Test to add interfaces to LAG ports - FAILED!'
        return True

    def showInterfaceLag(self):
        info('''
########## Test show interface lag command ##########
''')
        s1 = self.net.switches[0]

        # Verify 'show interface lag1' shows correct  information about lag1
        info('''
  ######## Verify show interface lag1 ########
''')
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

        # Verify 'show interface lag 2' shows correct error
        info('''
  ######## Verify show interface lag 2 ########
''')
        success = 0
        out = s1.cmdCLI('do show interface lag 2')
        lines = out.split('\n')
        for line in lines:
            if 'Aggregate-name lag 2 ' in line:
                success += 1
        assert success == 0,\
            'Test show interface lag 2 command - FAILED!'

        # Verify 'show interface lag' shows correct error
        info('''
  ######## Verify show interface lag  ########
''')
        success = 0
        out = s1.cmdCLI('do show interface lag')
        lines = out.split('\n')
        for line in lines:
            if 'Command incomplete' in line:
                success += 1
        assert success == 1,\
            'Test show interface lag command - FAILED!'

        return True


class Test_lacp_cli:

    def setup(self):
        pass

    def teardown(self):
        pass

    def setup_class(cls):
        Test_lacp_cli.test = LACPCliTest()

    def test_createLagPort(self):
        if self.test.createLagPort():
            info('''
########## Test to create LAG Port - SUCCESS! ##########
''')

    def test_addInterfacesToLags(self):
        if self.test.addInterfacesToLags():
            info('''
########## Test to add interfaces to LAG ports - SUCCESS! ##########
''')

    def test_showInterfaceLag(self):
        if self.test.showInterfaceLag():
            info('''
########## Test show interface lag command - SUCCESS! ##########
''')

    def teardown_class(cls):
        Test_lacp_cli.test.net.stop()

    def setup_method(self, method):
        pass

    def teardown_method(self, method):
        pass

    def __del__(self):
        del self.test
