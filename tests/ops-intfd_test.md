# ops-intfd Test Cases

[TOC]

##  user config ##
### Objective ###
Verify user configurable options work correctly.
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Set admin=up on interface
 - Verify "error=module\_missing"
2. Set admin=down
 - Verify "error=admin\_down"
 - Verify "hw\_enable=false"
3. Set all user config items
 - admin=down, autoneg=on, speeds=1000, duplex=full, pause=rxtx, mtu=1500
 - Verify "error=admin\_down"
4. Clear all user config items
 - Verify "error=admin\_down"
5. Clear MTU
 - Verify MTU=default mtu
6. Set MTU to minimum
 - Verify MTU=min mtu
7. Set MTU to maximum
 - Verify MTU=max mtu
8. Set MTU to below minimum
 - Verify "error=invalid\_mtu"
 - Verify MTU!=<below min value>
9. Clear all user config items
10. Set admin=up
11. Set speeds=<each valid value>
 -  Verify speed is set correctly
12. Set speeds=<invalid value>
 -  Verify error="invalid\_speeds"

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

##  pm info detect ##
### Objective ###
Verify that error is correctly set for various pm conditions.
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Set pm info to valid values
 - Verify error is not "module\_missing"
2. Set pm info to "absent"
 - Verify error="module\_missing"
3. Set pm info connector="unknown" and connector\_status="unrecognized"
 - Verify error="module\_unrecognized"
4. Set pm info connector="unknown" and connector\_status="unsupported"
 - Verify error="module\_unsupported"
5. Set pm info connector to a value unknown to intfd (such as SFP\_LX)
 - Verify error="module\_unsupported"
6. Set pm info connector to various valid types
 - Verify hw\_intf\_config is set properly
7. Set splittable parent port to "no-split"
8. Set parent pm info connector to valid values
 - Verify hw\_intf\_config is set properly
9. Set splittable parent port to "split"
10. Set child pm info connector to valid values
 -  Verify hw\_intf\_config is set properly
11. Set child pm info connector to invalid values
 -  Verify error="module\_unsupported"

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

##  user config autoneg  ##
### Objective ###
Verify that response to user configuration of autoneg is handled correctly based on pm type.
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Clear user config for SFP interface and QSFP interface (note: autoneg is not set)
2. Set SFP pm info to valid values
 - Verify that hw\_intf\_config:autoneg is set appropriately
3. Set QSFP parent port to "no-split"
4. Set QSFP pm info to valid values
 - Verify that hw\_intf\_config:autoneg is set appropriately
5. Set QSFP parent port to "split"
6. Set QSFP child pm info to valid values
 - Verify that hw\_intf\_config:autoneg is set appropriately
7. Clear user config for SFP interface and QSFP interface -
* CASE 1: AN not specified, user speeds not specified
* CASE 1a: AN supported or required
8. Set SFP connector="SFP\_RJ45"
 - Verify autoneg="on"
* CASE 1b: AN not supported
9. Set QSFP connector="QSFP\_SR4"
 - Verify autoneg="off"
* CASE 2: AN set to True, speeds not specified
* CASE 2a: AN supported or required
10. Set SFP connector="SFP\_RJ45"
11. Set SFP autoneg="on"
 -  Verify autoneg="on"
* CASE 2b: AN not supported
12. Set QSFP connector="QSFP\_SR4"
13. Set QSFP autoneg="on"
 -  Verify error="autoneg\_not\_support"
* CASE 3: AN set to False, speeds not specified
* CASE 3a: AN supported but not required
 - Note: We don't currently have a type like this, so no test for it.
* CASE 3b: AN required
14. Set SFP connector="SFP\_RJ45"
15. Set SFP autoneg="off"
 -  Verify error="autoneg\_required"
* CASE 3c: AN not supported
16. Set QSFP connector="QSFP\_LR4"
17. Set QSFP autoneg="off"
 -  Verify autoneg="off"
* CASE 4: AN not specified, speeds specified
* CASE 4a: AN supported or required
18. Clear user config for SFP interface
19. Set SFP connector="SFP\_RJ45", speeds=1000
 -  Verify autoneg="on", speeds=1000
* CASE 4b: AN not supported
20. Clear user config for QSFP interface
21. Set QSFP connector="QSFP\_LR4", speeds=40000
 -  Verify autoneg="off", speeds=40000
* CASE 5: AN set to True, speeds set
* CASE 5a: AN supported or required
22. Set SFP autoneg="on", connector="SFP\_RJ45", speeds=1000
 -  Verify autoneg="on", speeds=1000
* CASE 5b: AN not supported
23. Set QSFP connector="QSFP\_LR4"
24. Set QSFP autoneg="on"
 -  Verify error="autoneg\_not\_supported"
* CASE 6: AN set to False, speeds specified
* CASE 6a: AN required
25. Set SFP autoneg="off", connector="SFP\_RJ45", speeds=1000
* CASE 6b: AN not supported
26. Set QSFP connector="QSFP\_LR4", autoneg="off", speeds=40000
 -  Verify autoneg="off", speeds=40000

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

##  user config pause ##
### Objective ###
Verify that pause configuration is handled correctly
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Clear user configuration for SFP interface
2. Set pause to each of "none", "rx", "tx", "rxtx"
 - Verify hw\_intf\_config:pause is set accordingly

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

##  user config qsfp splitter ##
### Objective ###
Verify that QSFP splitter configuration is handled correctly
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Clear user configuration for QSFP interface
 - Verify parent error="admin\_down"
 - Verify child error="lanes\_not\_split"
2. Set QSFP parent to "no-split"
 - Verify children not up
3. Set QSFP children enable=true
 - Verify children not up
4. Set QSFP parent enable=true
 - Verify parent is up
5. Set QSFP parent to "split"
 - Verify parent is down, error="lanes\_split"
 - Verify the children are enabled
6. Set QSFP parent enable=false
 - Verify the children are enabled
7. Set QSFP parent to "no-split"
 - Verify children not up

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

##  fixed 1G fixed ports ##
### Objective ###
Verify fixed ports (not pluggable) are handled correctly
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
[h1]<-->[s1]
```
### Description ###
1. Read fixed port hw\_intf\_info
 - Verify pluggable="false", connector="RJ45", error="admin\_down"
2. Set fixed port admin="up"
 - Verify hw\_enable="true", autoneg="on", intf\_type="1GBASE\_T"

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.
