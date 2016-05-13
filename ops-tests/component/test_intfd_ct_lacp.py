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

TOPOLOGY = """
#
# +-------+
# |  sw1  |
# +-------+
#

# Nodes
[type=openswitch name="Switch 1"] sw1
"""


def test_interfaces_lag(topology, step):
    sw1 = topology.get('sw1')

    assert sw1 is not None

    step("### Add interfaces to LAG ports ###\n")
    interface_found_in_lag = False
    sw1('configure terminal')
    sw1('interface lag 1')
    sw1('interface 1')
    sw1('lag 1')
    sw1('interface 2')
    sw1('lag 1')
    out = sw1('do show lacp aggregates')
    lines = out.split('\n')
    for line in lines:
        if 'Aggregated-interfaces' in line and '1' in line and '2' in line:
            interface_found_in_lag = True

    assert interface_found_in_lag, \
        'Test to add interfaces to LAG ports - FAILED!'

    step("### Test show interface lag command ###\n")
    # Verify 'show interface lag1' shows correct  information about lag1
    step("### Verify show interface lag1 ###\n")
    success = 0
    out = sw1('do show interface lag1')
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
    assert success == 4, \
        'Test show interface lag1 command - FAILED!'

    # Verify 'show interface lag 1' shows correct error
    step("### Verify show interface lag 1 ###\n")
    success = 0
    out = sw1('do show interface lag 1')
    lines = out.split('\n')
    for line in lines:
        if 'Unknown command' in line:
            success += 1
    assert success == 1, \
        'Test show interface lag 1 command - FAILED!'

    step("### Test show running-config interface lag command ###\n")
    sw1("end")
    sw1('configure terminal')
    sw1('interface lag 1')
    sw1('ip address 10.1.1.1/24')
    sw1('ipv6 address 2001::1/12')
    sw1('end')

    # Get information from show running-config interface
    out = sw1('show running-config interface')
    lines = out.split('\n')
    success = 0
    for line in lines:
        if 'ip address 10.1.1.1/24' in line:
            success += 1
        if 'ipv6 address 2001::1/12' in line:
            success += 1
    assert success == 2, \
        'Test show running-config interface command - FAILED!'

    out = sw1('show running-config interface lag1')
    lines = out.split('\n')
    success = 0
    for line in lines:
        if 'ip address 10.1.1.1/24' in line:
            success += 1
        if 'pv6 address 2001::1/12' in line:
            success += 1
    assert success == 2, \
        'Test show running-config interface <lag_id> - FAILED!'

    step("########## Test interface lagXX command ##########\n")
    # Verify 'show interface lag 1' shows correct error
    print("  ######## Verify interface lag1 ########\n")
    success = 0
    out = sw1('interface lag1')
    lines = out.split('\n')
    for line in lines:
        if 'Unknown command' in line:
            success += 1
    assert success == 1, \
        'Test interface lag1 command - FAILED!'
