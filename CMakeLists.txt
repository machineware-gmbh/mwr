 ##############################################################################
 #                                                                            #
 # Copyright 2022 MachineWare GmbH                                            #
 # All Rights Reserved                                                        #
 #                                                                            #
 # This is unpublished proprietary work and may not be used or disclosed to   #
 # third parties, copied or duplicated in any form, in whole or in part,      #
 # without prior written permission of the authors.                           #
 #                                                                            #
 ##############################################################################

cmake_minimum_required(VERSION 3.11)
project(mwr VERSION 2022.10.13 LANGUAGES CXX)

set(MWR_LINTER "" CACHE STRING "Code linter to use")
set(MWR_BUILD_TESTS ON CACHE BOOL "Build unit tests")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(GenVersionInfo)
include(Sanitizer)

find_package(Threads REQUIRED)

set(src ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(inc ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(gen ${CMAKE_CURRENT_BINARY_DIR}/gen)

configure_file(${src}/mwr/version.h.in
               ${gen}/mwr/version.h @ONLY)

add_library(mwr STATIC
            ${src}/mwr/strings.cpp
            ${src}/mwr/utils.cpp)

target_include_directories(mwr PUBLIC ${inc})
target_include_directories(mwr PUBLIC ${gen})
target_compile_options(mwr PRIVATE -Wall -Werror)
target_compile_features(mwr PRIVATE cxx_std_17)
target_compile_definitions(mwr PUBLIC $<$<CONFIG:DEBUG>:MWR_DEBUG>)
target_link_libraries(mwr Threads::Threads)
target_link_libraries(mwr m)
set_target_properties(mwr PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(mwr PROPERTIES CXX_CLANG_TIDY "${MWR_LINTER}")
set_target_properties(mwr PROPERTIES VERSION "${MWR_VERSION}")
set_target_properties(mwr PROPERTIES SOVERSION "${MWR_VERSION_MAJOR}")

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    install(TARGETS mwr DESTINATION lib)
    install(DIRECTORY ${inc}/ DESTINATION include)
    install(DIRECTORY ${gen}/ DESTINATION include)

    if(MWR_BUILD_TESTS)
        enable_testing()
        add_subdirectory(test)
    endif()
endif()

