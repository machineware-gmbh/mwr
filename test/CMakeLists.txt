 ##############################################################################
 #                                                                            #
 # Copyright (C) 2022 MachineWare GmbH                                        #
 # All Rights Reserved                                                        #
 #                                                                            #
 # This is work is licensed under the terms described in the LICENSE file     #
 # found in the root directory of this source tree.                           #
 #                                                                            #
 ##############################################################################

if (NOT TARGET gtest OR NOT TARGET gmock)
    option(INSTALL_GTEST OFF)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
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
add_subdirectory(logging)
add_subdirectory(utils)
