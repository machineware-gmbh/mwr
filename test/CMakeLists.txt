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

if (NOT TARGET gtest OR NOT TARGET gmock)
    option(INSTALL_GTEST OFF)
    add_subdirectory(googletest EXCLUDE_FROM_ALL)
endif()

set(resources ${CMAKE_CURRENT_SOURCE_DIR}/resources)

if(CMAKE_BUILD_TYPE MATCHES "ASAN")
    string(APPEND ENVVARS "ASAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/sanitizer/asan.cfg:detect_leaks=1:fast_unwind_on_malloc=0;")
    string(APPEND ENVVARS "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/sanitizer/lsan.cfg;")
elseif(CMAKE_BUILD_TYPE MATCHES "TSAN")
    string(APPEND ENVVARS "TSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/sanitizer/tsan.cfg;")
elseif(CMAKE_BUILD_TYPE MATCHES "UBSAN")
    string(APPEND ENVVARS "UBSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/sanitizer/ubsan.cfg:print_stacktrace=1;")
endif()

add_subdirectory(core)
add_subdirectory(stl)
add_subdirectory(utils)
