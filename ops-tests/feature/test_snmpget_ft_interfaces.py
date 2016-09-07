# -*- coding: utf-8 -*-
# (C) Copyright 2015 Hewlett Packard Enterprise Development LP
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
OpenSwitch Test for interface table related SNMP operations.
"""

from pytest import mark
from time import sleep


TOPOLOGY = """
# +-------+
# |       |     +--------+
# |  hs1  <----->  ops1  |
# |       |     +--------+
# +-------+

# Nodes
[type=openswitch name="OpenSwitch 1"] ops1
[type=host name="Host 1"] hs1

# Ports
[force_name=oobm] ops1:sp1

# Links
hs1:1 -- ops1:sp1
"""


def snmp_get(ops1_or_hs1, version, accesscontrol, host, oid,
             extrav3conf=None):
    if version is "v1":
        retstruct = ops1_or_hs1("snmpget -v1 -c" + accesscontrol + " "
                                "" + host + " " + oid + " ", shell='bash')
        return retstruct
    elif version is "v2c":
        retstruct = ops1_or_hs1("snmpget -v2c -c" + accesscontrol + " "
                                "" + host + " " + oid + " ", shell='bash')
        return retstruct
    elif version is "v3":
        retstruct = ops1_or_hs1("snmpget -v3 " + extrav3conf + " "
                                "" + host + " " + oid + " ", shell='bash')
        return retstruct


def snmpget_v1_test_local(ops1):
    retstruct = snmp_get(ops1, "v1", "public", "localhost", "IF-MIB"
                         "::ifOutDiscards.1")
    assert "IF-MIB::ifOutDiscards.1 = Counter32: 106" in retstruct, "snmpget"
    " v1 failed"
    retstruct = snmp_get(ops1, "v1", "public", "localhost", "IF-MIB"
                         "::ifOutUcastPkts.1")
    assert "IF-MIB::ifOutUcastPkts.1 = Counter32: 10003" in retstruct, "snmp"
    "get v1 failed"
    retstruct = snmp_get(ops1, "v1", "public", "localhost", "IF-MIB"
                         "::ifOutOctets.1")
    assert "IF-MIB::ifOutOctets.1 = Counter32: 100004" in retstruct, "snmpget"
    " v1 failed"
    retstruct = snmp_get(ops1, "v1", "public", "localhost", "IF-MIB"
                         "::ifInDiscards.1")
    assert "IF-MIB::ifInDiscards.1 = Counter32: 105" in retstruct, "snmpget"
    " v1 failed"
    retstruct = snmp_get(ops1, "v1", "public", "localhost", "IF-MIB"
                         "::ifInUcastPkts.1")
    assert "IF-MIB::ifInUcastPkts.1 = Counter32: 100002" in retstruct, "snmp"
    "get v1 failed"
    retstruct = snmp_get(ops1, "v1", "public", "localhost", "IF-MIB"
                         "::ifInOctets.1")
    assert "IF-MIB::ifInOctets.1 = Counter32: 10001" in retstruct, "snmpget"
    " v1 failed"
    retstruct = snmp_get(ops1, "v1", "public", "localhost", "IF-MIB"
                         "::ifInErrors.1")
    assert "IF-MIB::ifInErrors.1 = Counter32: 107" in retstruct, "snmpget"
    " v1 failed"
    retstruct = snmp_get(ops1, "v1", "public", "localhost", "IF-MIB"
                         "::ifOutErrors.1")
    assert "IF-MIB::ifOutErrors.1 = Counter32: 108" in retstruct, "snmpget"
    " v1 failed"

    for line in retstruct:
        assert "Error in packet" not in line, "General failure accured"
        " in snmpget"
    for line in retstruct:
        assert "Timeout: No Response" not in line, "snmpget timed out"


def snmpget_v2c_test_local(ops1):

    retstruct = snmp_get(ops1, "v2c", "public", "localhost", "IF-MIB"
                         "::ifOutDiscards.1")
    assert "IF-MIB::ifOutDiscards.1 = Counter32: 106" in retstruct, "snmpget"
    " v1 failed"
    retstruct = snmp_get(ops1, "v2c", "public", "localhost", "IF-MIB"
                         "::ifOutUcastPkts.1")
    assert "IF-MIB::ifOutUcastPkts.1 = Counter32: 10003" in retstruct, "snmp"
    "get v1 failed"
    retstruct = snmp_get(ops1, "v2c", "public", "localhost", "IF-MIB"
                         "::ifOutOctets.1")
    assert "IF-MIB::ifOutOctets.1 = Counter32: 100004" in retstruct, "snmpget"
    " v1 failed"
    retstruct = snmp_get(ops1, "v2c", "public", "localhost", "IF-MIB"
                         "::ifInDiscards.1")
    assert "IF-MIB::ifInDiscards.1 = Counter32: 105" in retstruct, "snmpget"
    " v1 failed"
    retstruct = snmp_get(ops1, "v2c", "public", "localhost", "IF-MIB"
                         "::ifInUcastPkts.1")
    assert "IF-MIB::ifInUcastPkts.1 = Counter32: 100002" in retstruct, "snmp"
    "get v1 failed"
    retstruct = snmp_get(ops1, "v2c", "public", "localhost", "IF-MIB"
                         "::ifInOctets.1")
    assert "IF-MIB::ifInOctets.1 = Counter32: 10001" in retstruct, "snmpget"
    " v1 failed"
    retstruct = snmp_get(ops1, "v2c", "public", "localhost", "IF-MIB"
                         "::ifInErrors.1")
    assert "IF-MIB::ifInErrors.1 = Counter32: 107" in retstruct, "snmpget"
    " v1 failed"
    retstruct = snmp_get(ops1, "v2c", "public", "localhost", "IF-MIB"
                         "::ifOutErrors.1")
    assert "IF-MIB::ifOutErrors.1 = Counter32: 108" in retstruct, "snmpget"
    " v1 failed"

    for line in retstruct:
        assert "Error in packet" not in line, "General failure accured"
        " in snmpget"

    for line in retstruct:
        assert "Timeout: No Response" not in line, "snmpget timed out"


def snmpget_v3_test_local(ops1):

    retstruct = snmp_get(ops1, "v3", "None", "localhost",
                         "IF-MIB::ifOutDiscards.1",
                         "-u testv3user -l authNoPriv -a md5 -A password")
    assert "IF-MIB::ifOutDiscards.1 = Counter32: 106" in retstruct, "snmpget"
    " v3 failed"

    retstruct = snmp_get(ops1, "v3", "None", "localhost",
                         "IF-MIB::ifOutUcastPkts.1",
                         "-u testv3user -l authNoPriv -a md5 -A password")
    assert "IF-MIB::ifOutUcastPkts.1 = Counter32: 10003" in retstruct, "snmp"
    "get v3 failed"

    retstruct = snmp_get(ops1, "v3", "None", "localhost",
                         "IF-MIB::ifOutOctets.1",
                         "-u testv3user -l authNoPriv -a md5 -A password")
    assert "IF-MIB::ifOutOctets.1 = Counter32: 100004" in retstruct, "snmpget"
    " v3 failed"

    retstruct = snmp_get(ops1, "v3", "None", "localhost",
                         "IF-MIB::ifInDiscards.1",
                         "-u testv3user -l authNoPriv -a md5 -A password")
    assert "IF-MIB::ifInDiscards.1 = Counter32: 105" in retstruct, "snmpget"
    " v3 failed"

    retstruct = snmp_get(ops1, "v3", "None", "localhost",
                         "IF-MIB::ifInUcastPkts.1",
                         "-u testv3user -l authNoPriv -a md5 -A password")
    assert "IF-MIB::ifInUcastPkts.1 = Counter32: 100002" in retstruct, "snmp"
    "get v3 failed"

    retstruct = snmp_get(ops1, "v3", "None", "localhost",
                         "IF-MIB::ifInOctets.1",
                         "-u testv3user -l authNoPriv -a md5 -A password")
    assert "IF-MIB::ifInOctets.1 = Counter32: 10001" in retstruct, "snmpget"
    " v3 failed"

    retstruct = snmp_get(ops1, "v3", "None", "localhost",
                         "IF-MIB::ifInErrors.1",
                         "-u testv3user -l authNoPriv -a md5 -A password")
    assert "IF-MIB::ifInErrors.1 = Counter32: 107" in retstruct, "snmpget"
    " v3 failed"

    retstruct = snmp_get(ops1, "v3", "None", "localhost",
                         "IF-MIB::ifOutErrors.1",
                         "-u testv3user -l authNoPriv -a md5 -A password")
    assert "IF-MIB::ifOutErrors.1 = Counter32: 108" in retstruct, "snmpget"
    " v3 failed"

    for line in retstruct:
        assert "Error in packet" not in line, "General failure accured"
        " in snmpget"

    for line in retstruct:
        assert "Timeout: No Response" not in line, "snmpget timed out"


def config(ops1, hs1):

    # Configure all the interface statistics using ovs-vsctl

    output = ops1('list system', shell='vsctl')
    lines = output.split('\n')
    for line in lines:
        if '_uuid' in line:
            _id = line.split(':')
            sys_uuid = _id[1].strip()

    ops1("set system "+sys_uuid+" other_config:"
         "stats-update-interval=300000000", shell='vsctl')

    sleep(10)

    ops1("set interface 1 statistics:rx_bytes=10001", shell='vsctl')
    ops1("set interface 1 statistics:rx_packets=100002", shell='vsctl')
    ops1("set interface 1 statistics:tx_packets=10003", shell='vsctl')
    ops1("set interface 1 statistics:tx_bytes=100004", shell='vsctl')
    ops1("set interface 1 statistics:rx_dropped=105", shell='vsctl')
    ops1("set interface 1 statistics:tx_dropped=106", shell='vsctl')
    ops1("set interface 1 statistics:rx_errors=107", shell='vsctl')
    ops1("set interface 1 statistics:tx_errors=108", shell='vsctl')

    ops1("set interface 2 statistics:rx_bytes=20001", shell='vsctl')
    ops1("set interface 2 statistics:rx_packets=20002", shell='vsctl')
    ops1("set interface 2 statistics:tx_packets=20003", shell='vsctl')
    ops1("set interface 2 statistics:tx_bytes=20004", shell='vsctl')
    ops1("set interface 2 statistics:rx_dropped=205", shell='vsctl')
    ops1("set interface 2 statistics:tx_dropped=206", shell='vsctl')
    ops1("set interface 2 statistics:rx_errors=207", shell='vsctl')
    ops1("set interface 2 statistics:tx_errors=208", shell='vsctl')

    # Configure auth user - testv3user
    with ops1.libs.vtysh.Configure() as ctx:
        ctx.snmpv3_user_auth_auth_pass('testv3user', auth_protocol='md5',
                                       auth_password='password')
    result = ops1.libs.vtysh.show_snmpv3_users()
    assert 'testv3user' in result
    assert result['testv3user']['AuthMode'] == 'md5'

    # Configure static IP for management I/F
    with ops1.libs.vtysh.ConfigInterfaceMgmt() as ctx:
        ctx.ip_static("10.10.10.4/24")
    ops1.libs.vtysh.show_running_config()
    # Configure host interfaces
    hs1.libs.ip.interface('1', addr='10.10.10.5/24', up=True)

    sleep(3)
    ping = hs1.libs.ping.ping(1, '10.10.10.4')
    assert ping['transmitted'] == ping['received']


def unconfig(ops1, hs1):

    # Configure auth user - testv3user
    with ops1.libs.vtysh.Configure() as ctx:
        ctx.no_snmpv3_user_auth_auth_pass('testv3user', auth_protocol='md5',
                                          auth_password='password')


def snmpget_v1_test_remote(hs1):
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.10.1")
    assert "iso.3.6.1.2.1.2.2.1.10.1 = Counter32: 1000"
    "1" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.11.1")
    assert "iso.3.6.1.2.1.2.2.1.11.1 = Counter32: 10000"
    "2" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.13.1")
    assert "iso.3.6.1.2.1.2.2.1.13.1 = Counter32: 10"
    "5" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.16.1")
    assert "iso.3.6.1.2.1.2.2.1.16.1 = Counter32: 10000"
    "4" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.17.1")
    assert "iso.3.6.1.2.1.2.2.1.17.1 = Counter32: 1000"
    "3" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.19.1")
    assert "iso.3.6.1.2.1.2.2.1.19.1 = Counter32: 10"
    "6" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.14.1")
    assert "iso.3.6.1.2.1.2.2.1.14.1 = Counter32: 10"
    "7" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.20.1")
    assert "iso.3.6.1.2.1.2.2.1.20.1 = Counter32: 10"
    "8" not in retstruct, "V1 snmpget from remote host failed"
    for line in retstruct:
        assert "Error in packet" not in line, "General failure accured"
        " in snmpget"
    for line in retstruct:
        assert "Timeout: No Response" not in line, "snmpget imed out"


def snmpget_v2c_test_remote(hs1):
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.10.1")
    assert "iso.3.6.1.2.1.2.2.1.10.1 = Counter32: 1000"
    "1" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.11.1")
    assert "iso.3.6.1.2.1.2.2.1.11.1 = Counter32: 10000"
    "2" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.13.1")
    assert "iso.3.6.1.2.1.2.2.1.13.1 = Counter32: 10"
    "5" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.16.1")
    assert "iso.3.6.1.2.1.2.2.1.16.1 = Counter32: 10000"
    "4" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.17.1")
    assert "iso.3.6.1.2.1.2.2.1.17.1 = Counter32: 1000"
    "3" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.19.1")
    assert "iso.3.6.1.2.1.2.2.1.19.1 = Counter32: 10"
    "6" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.14.1")
    assert "iso.3.6.1.2.1.2.2.1.14.1 = Counter32: 10"
    "7" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v1 -cpublic 10.10.10.4:161 .1.3.6.1.2.1.2."
                    "2.1.20.1")
    assert "iso.3.6.1.2.1.2.2.1.20.1 = Counter32: 10"
    "8" not in retstruct, "V1 snmpget from remote host failed"
    for line in retstruct:
        assert "Error in packet" not in line, "General failure accured"
        " in snmpget"
    for line in retstruct:
        assert "Timeout: No Response" not in line, "snmpget timed out"


def snmpget_v3_test_remote(ops1, hs1):
    retstruct = hs1("snmpget -v3 -u testv3user -l authNoPriv "
                    "-a md5 -A password 10.10.10.4:161"
                    " .1.3.6.1.2.1.2.2.1.10.1")
    assert "iso.3.6.1.2.1.2.2.1.10.1 = Counter32: 1000"
    "1" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v3 -u testv3user -l authNoPriv "
                    "-a md5 -A password 10.10.10.4:161"
                    " .1.3.6.1.2.1.2.2.1.11.1")
    assert "iso.3.6.1.2.1.2.2.1.11.1 = Counter32: 10000"
    "2" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v3 -u testv3user -l authNoPriv "
                    "-a md5 -A password 10.10.10.4:161"
                    " .1.3.6.1.2.1.2.2.1.13.1")
    assert "iso.3.6.1.2.1.2.2.1.13.1 = Counter32: 10"
    "5" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v3 -u testv3user -l authNoPriv "
                    "-a md5 -A password 10.10.10.4:161"
                    " .1.3.6.1.2.1.2.2.1.16.1")
    assert "iso.3.6.1.2.1.2.2.1.16.1 = Counter32: 10000"
    "4" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v3 -u testv3user -l authNoPriv "
                    "-a md5 -A password 10.10.10.4:161"
                    " .1.3.6.1.2.1.2.2.1.17.1")
    assert "iso.3.6.1.2.1.2.2.1.17.1 = Counter32: 1000"
    "3" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v3 -u testv3user -l authNoPriv "
                    "-a md5 -A password 10.10.10.4:161"
                    " .1.3.6.1.2.1.2.2.1.19.1")
    assert "iso.3.6.1.2.1.2.2.1.19.1 = Counter32: 10"
    "6" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v3 -u testv3user -l authNoPriv "
                    "-a md5 -A password 10.10.10.4:161"
                    " .1.3.6.1.2.1.2.2.1.14.1")
    assert "iso.3.6.1.2.1.2.2.1.14.1 = Counter32: 10"
    "7" not in retstruct, "V1 snmpget from remote host failed"
    retstruct = hs1("snmpget -v3 -u testv3user -l authNoPriv "
                    "-a md5 -A password 10.10.10.4:161"
                    " .1.3.6.1.2.1.2.2.1.20.1")
    assert "iso.3.6.1.2.1.2.2.1.20.1 = Counter32: 10"
    "8" not in retstruct, "V1 snmpget from remote host failed"
    for line in retstruct:
        assert "Error in packet" not in line, "General failure accured"
        " in snmpget"
    for line in retstruct:
        assert "Timeout: No Response" not in line, "snmpget timed out"


@mark.platform_incompatible(['docker'])
def test_snmpget_ft_interfaces(topology, step):
    ops1 = topology.get("ops1")
    hs1 = topology.get("hs1")
    config(ops1, hs1)
    sleep(30)
    snmpget_v1_test_local(ops1)
    snmpget_v2c_test_local(ops1)
    snmpget_v3_test_local(ops1)
    snmpget_v1_test_remote(hs1)
    snmpget_v2c_test_remote(hs1)
    snmpget_v3_test_remote(ops1, hs1)
    unconfig(ops1, hs1)
