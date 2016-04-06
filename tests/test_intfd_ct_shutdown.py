#!/usr/bin/python

# Copyright (C) 2015-2016 Hewlett Packard Enterprise Development LP
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


class adminstateupdateCTTest(OpsVsiTest):

    def setupNet(self):
        self.net = Mininet(topo=SingleSwitchTopo(k=0,
                                                 hopts=self.getHostOpts(),
                                                 sopts=self.getSwitchOpts()),
                           switch=VsiOpenSwitch,
                           host=OpsVsiHost,
                           link=OpsVsiLink, controller=None,
                           build=True)

    def test_lag_shutdown(self):

        info('\n########## Test shutdown and no shutdown for LAG'
             ' ##########\n')
        n_intfs = 4
        s1 = self.net.switches[0]
        s1.cmdCLI('configure terminal')
        s1.cmdCLI('interface lag 1')
        s1.cmdCLI('exit')

        for intf_num in range(1, n_intfs):
            s1.cmdCLI('interface %d' % intf_num)
            s1.cmdCLI('lag 1')
            s1.cmdCLI('exit')

        s1.cmdCLI('interface lag 1')
        s1.cmdCLI('no shutdown')
        s1.cmdCLI('end')

        out = s1.cmdCLI('show running-config interface')

        total_lag = 0
        lines = out.split("\n")

        for line in lines:
            if 'no shutdown' in line:
                total_lag += 1

        # total_lag should be the no shutdowns of intfs + lag +
        # default vlan
        assert total_lag is 5, \
            "Failed test, all interfaces are not up!"

        s1.cmdCLI('configure terminal')
        s1.cmdCLI('interface lag 1')
        s1.cmdCLI('shutdown')
        s1.cmdCLI('end')

        out = s1.cmdCLI('show running-config interface')

        total_lag = 0
        lines = out.split("\n")

        for line in lines:
            if 'no shutdown' in line:
                total_lag += 1

        # total_lag should be the no shutdown of the default vlan
        assert total_lag is 1, \
            "Failed test, all interfaces are not down!"

@pytest.mark.skipif(True, reason="Disabling old tests")
class Test_portd_admin_state_update:

    def setup_class(cls):
        Test_portd_admin_state_update.test = adminstateupdateCTTest()

    def teardown_class(cls):
        # Stop the Docker containers, and
        # mininet topology
        Test_portd_admin_state_update.test.net.stop()

    def test_lag_shutdown(self):
        self.test.test_lag_shutdown()
        info('''
########## Test show lacp interface command - SUCCESS! ##########
''')

    def __del__(self):
        del self.test
