High level design of ops-intfd
==============================

The ops-intfd process monitors and manages rows in the Interface database table.

Reponsibilities
---------------
The ops-intfd process is responsible for examining the physical and configuration information for each interface in the Interface table, determining if there are any conflicts in the configuration, and calculating (and updating) the hardware configuration for each interface.

Design choices
--------------
The ops-intfd was added to the OpenSwitch architecture so that there would be a single entity responsible for coallescing the information about the interface into the correct hardware configuration. If multiple daemons had joint responsibility, the design and implementation would have been considerably more complex.

Alternately, the business logic in ops-intfd could have been incorporated into vswitchd.

Relationships to external OpenSwitch entities
---------------------------------------------
```ditaa
+--------+
|database+-----------+
+-+-^----+           |
  | |                |
  | |                |
+-v-+-----+    +-----v-----+
|ops-intfd|    |ops-switchd|
+---------+    +-----+-----+
                     |
                     |
                 +---v--+
                 |SWITCH|
                 +------+
```

The ops-intfd process monitors the Interface table in the database for changes. It examines each row to determine if the physical interface and the user configuration are in conflict and calculates the appropriate hardware configuration for the interface. It writes the resulting information back into the interface row, which is then consumed by the ops-switchd process to configure the switch.

OVSDB-Schema
------------
The ops-intfd process examines the `other_info:max_transmission_unit` field in the Subsystem table to determine the hardware MTU limit for the switch.

The ops-intfd process examines the following columns in the Interface table rows:

* user\_config
  * admin
    This is set to "up" when the user has enabled the interface.
  * autoneg
    Enable/disable autonegotiation on the interface.
  * pause
    Specify how pause is to be configured on the interface.
  * duplex
    Specify full or half duplex.
  * speeds
    Specify the speeds that should be advertised during autonegotiation.
  * mtu
    Limit the MTU for the interface.
  * lane\_split
    The user sets this to "true" if this is a splittable QSFP+ interface and the user has inserted a splitter QSFP+ module (or intends to do so).
* pm\_info
  * connector\_status
    The ops-pmd process determines if the interface is pluggable, and if there is a pluggable module is inserted, and if the module inserted is compatible. The ops-intfd process uses this information to determine if the pluggable module presence and status is an impediment to enabling the interface.
  * connector
    The ops-pmd process determines what type of connector and module (if any) are present. The ops-intfd uses this information to determine rules for what configuration values are incompatible with the hardware.
* split\_parent
  The ops-intfd process examines the `split_parent` value so that it can locate the "parent" interface for an interface that is the individual 10-Gb interface in a QSFP+ module. It needs this information so that it can verify if the parent interface has been configured by the user for "split" operation. The user must configure the parent interface for "split" operation in order for any configuration of the "child" port to be considered valid.
* split\_children
  The ops-intfd process examines the `split_children` to determine all of the "split" interfaces for a QSFP+ module.
* hw\_intf\_info
  * pluggable
    The ops-intfd process examines this to determine if the data in pm\_info is of any interest.
  * connector
    If the interface does not support pluggable modules, this value is used to determine constraints on the configuration.
  * speeds
    This value is compared to the user-configured speed(s) to verify that the user has not specified values that are not valid for the interface.
  * max\_speed
    The ops-intfd process verifies that the `max_speed` value is present for the interface.

Internal structure
------------------

* initialization
  Subscribe to database tables and columns, and general initialization
* main loop
  * reconfigure
    * process interface additions and deletions
      Future: modular switches where interfaces may be added/removed dynamically
    * handle interface configuration modifications
      * process parent-child relationships
        If splittable, make sure that the internal linkage between the parent and child interfaces is established.
      * parse user\_config, pm\_info, other data
        Pull the data out of the IDL and cache it in internal data structures.
      * set interface configuration
        * verify user settings against hardware capabilities
          Determine if there are conflicts between the hardware and the user configuration.
        * set hardware configuration
          Write the hardware configuration into the database, where it can be used by ops-switchd to configure the switch.

References
----------
* [pluggable module feature](http://www.openswitch.net/documents/dev/pluggable_modules_design)
