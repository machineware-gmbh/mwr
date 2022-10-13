 ##############################################################################
 #                                                                            #
 # Copyright 2022 MachineWare GmbH                                            #
 #                                                                            #
 # Licensed under the Apache License, Version 2.0 (the "License");            #
 # you may not use this file except in compliance with the License.           #
 # You may obtain a copy of the License at                                    #
 #                                                                            #
 #     http://www.apache.org/licenses/LICENSE-2.0                             #
 #                                                                            #
 # Unless required by applicable law or agreed to in writing, software        #
 # distributed under the License is distributed on an "AS IS" BASIS,          #
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
 # See the License for the specific language governing permissions and        #
 # limitations under the License.                                             #
 #                                                                            #
 ##############################################################################

find_package(PkgConfig QUIET)
pkg_check_modules(PKGCFG_LIBEPOXY QUIET epoxy)

find_path(LIBEPOXY_INCLUDE_DIRS NAMES "epoxy/gl.h"
          HINTS $ENV{LIBEPOXY_HOME}/include ${PKGCFG_LIBEPOXY_INCLUDE_DIRS})

find_library(LIBEPOXY_LIBRARIES NAMES "epoxy"
             HINTS $ENV{LIBEPOXY_HOME}/lib ${PKGCFG_LIBEPOXY_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibEpoxy
    REQUIRED_VARS LIBEPOXY_INCLUDE_DIRS LIBEPOXY_LIBRARIES
    VERSION_VAR   PKGCFG_LIBEPOXY_VERSION)
mark_as_advanced(LIBEPOXY_INCLUDE_DIRS LIBEPOXY_LIBRARIES)

message(DEBUG "LIBEPOXY_FOUND        " ${LIBEPOXY_FOUND})
message(DEBUG "LIBEPOXY_INCLUDE_DIRS " ${LIBEPOXY_INCLUDE_DIRS})
message(DEBUG "LIBEPOXY_LIBRARIES    " ${LIBEPOXY_LIBRARIES})
