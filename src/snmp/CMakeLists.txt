# (C) Copyright 2016 Hewlett Packard Enterprise Development LP
#
#  Licensed under the Apache License, Version 2.0 (the &quot;License&quot;); you may
#  not use this file except in compliance with the License. You may obtain
#  a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an &quot;AS IS&quot; BASIS, WITHOUT
#  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#  License for the specific language governing permissions and limitations
#  under the License.

cmake_minimum_required (VERSION 2.8)

project ('intfd_snmp')

set (INCL_DIR ${CMAKE_SOURCE_DIR}/include)
set (LIBINTFDSNMP intfd_snmp)

# This option is passed by build system.
OPTION( CPU_LITTLE_ENDIAN "Specifies CPU architecture is Little-Endian" OFF )

# Define compile flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Werror")


add_definitions(-DHAVE_CONFIG_H -DHAVE_SOCKLEN_T)

# Rules to locate needed libraries
include(FindPkgConfig)
pkg_check_modules(OVSCOMMON REQUIRED libovscommon)
pkg_check_modules(OVSDB REQUIRED libovsdb)

include_directories (${INCL_DIR}
                     ${PROJECT_SOURCE_DIR}
                     ${OVSCOMMON_INCLUDE_DIRS}
                    )

# SNMP libraries source files
set (SOURCES_SNMP ${PROJECT_SOURCE_DIR}/IF_MIB_custom.c
                 ${PROJECT_SOURCE_DIR}/IF_MIB_plugins.c
                 ${PROJECT_SOURCE_DIR}/IF_MIB_scalars.c
                 ${PROJECT_SOURCE_DIR}/IF_MIB_scalars_ovsdb_get.c
                 ${PROJECT_SOURCE_DIR}/ifTable.c ${PROJECT_SOURCE_DIR}/ifXTable.c
                 ${PROJECT_SOURCE_DIR}/ifTable_data_access.c ${PROJECT_SOURCE_DIR}/ifXTable_data_access.c
                 ${PROJECT_SOURCE_DIR}/ifTable_data_get.c ${PROJECT_SOURCE_DIR}/ifXTable_data_get.c
                 ${PROJECT_SOURCE_DIR}/ifTable_interface.c ${PROJECT_SOURCE_DIR}/ifXTable_interface.c
                 ${PROJECT_SOURCE_DIR}/ifTable_ovsdb_get.c ${PROJECT_SOURCE_DIR}/ifXTable_ovsdb_get.c
    )


add_library (${LIBINTFDSNMP} SHARED ${SOURCES_SNMP})

# Installation
install(TARGETS ${LIBINTFDSNMP}
        LIBRARY DESTINATION lib/snmp/plugins
       )
