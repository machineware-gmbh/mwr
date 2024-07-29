 ##############################################################################
 #                                                                            #
 # Copyright (C) 2023 MachineWare GmbH                                        #
 # All Rights Reserved                                                        #
 #                                                                            #
 # This is work is licensed under the terms described in the LICENSE file     #
 # found in the root directory of this source tree.                           #
 #                                                                            #
 ##############################################################################

cmake_minimum_required(VERSION 3.11)
project(mwr VERSION 2024.07.29 LANGUAGES CXX)

set(MWR_LINTER "" CACHE STRING "Code linter to use")
set(MWR_COVERAGE OFF CACHE BOOL "Collect code coverage data")
set(MWR_BUILD_TESTS OFF CACHE BOOL "Build unit tests")
set(MWR_USE_LIBELF ON CACHE BOOL "Use libelf for reading ELF files")

include(cmake/common.cmake)
find_package(Threads REQUIRED)

if(MWR_USE_LIBELF)
    find_package(LibELF)
endif()

set(src ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(inc ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(gen ${CMAKE_CURRENT_BINARY_DIR}/gen)

configure_file(${src}/mwr/core/version.h.in
               ${gen}/mwr/core/version.h @ONLY)

add_library(mwr STATIC
            ${src}/mwr/core/utils.cpp
            ${src}/mwr/core/report.cpp
            ${src}/mwr/core/bitops.cpp
            ${src}/mwr/stl/strings.cpp
            ${src}/mwr/stl/threads.cpp
            ${src}/mwr/logging/publisher.cpp
            ${src}/mwr/logging/publishers/file.cpp
            ${src}/mwr/logging/publishers/stream.cpp
            ${src}/mwr/logging/publishers/terminal.cpp
            ${src}/mwr/logging/logger.cpp
            ${src}/mwr/utils/license.cpp
            ${src}/mwr/utils/modules.cpp
            ${src}/mwr/utils/options.cpp
            ${src}/mwr/utils/srec.cpp
            ${src}/mwr/utils/terminal.cpp
            ${src}/mwr/utils/uimage.cpp)

target_include_directories(mwr PUBLIC ${inc})
target_include_directories(mwr PUBLIC ${gen})

target_compile_options(mwr PRIVATE ${MWR_COMPILER_WARN_FLAGS})
target_compile_features(mwr PUBLIC cxx_std_17)
target_compile_definitions(mwr PUBLIC $<$<CONFIG:DEBUG>:MWR_DEBUG>)
target_link_libraries(mwr PUBLIC Threads::Threads)
target_link_libraries(mwr PUBLIC ${CMAKE_DL_LIBS})
set_target_properties(mwr PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(mwr PROPERTIES CXX_CLANG_TIDY "${MWR_LINTER}")
set_target_properties(mwr PROPERTIES VERSION "${MWR_VERSION}")
set_target_properties(mwr PROPERTIES SOVERSION "${MWR_VERSION_MAJOR}")

if(MSVC)
    target_sources(mwr PRIVATE ${src}/mwr/utils/aio_win32.cpp)
    target_sources(mwr PRIVATE ${src}/mwr/utils/library_win32.cpp)
    target_sources(mwr PRIVATE ${src}/mwr/utils/memory_win32.cpp)
    target_sources(mwr PRIVATE ${src}/mwr/utils/socket_win32.cpp)
    target_link_options(mwr PUBLIC /OPT:NOREF)
    target_link_libraries(mwr PUBLIC Dbghelp.lib)
    target_link_libraries(mwr PUBLIC ws2_32)
else()
    target_sources(mwr PRIVATE ${src}/mwr/utils/aio_posix.cpp)
    target_sources(mwr PRIVATE ${src}/mwr/utils/library_posix.cpp)
    target_sources(mwr PRIVATE ${src}/mwr/utils/memory_posix.cpp)
    target_sources(mwr PRIVATE ${src}/mwr/utils/socket_posix.cpp)
    target_link_libraries(mwr PUBLIC -rdynamic)
    if(NOT APPLE)
        target_link_libraries(mwr PUBLIC rt stdc++fs)
    endif()
endif()

if(LIBELF_FOUND)
    message(STATUS "Using libelf for reading ELF files")
    target_sources(mwr PRIVATE ${src}/mwr/utils/elf_libelf.cpp)
    target_include_directories(mwr SYSTEM PRIVATE ${LIBELF_INCLUDE_DIRS})
    target_link_libraries(mwr PUBLIC ${LIBELF_LIBRARIES})
else()
    message(STATUS "Using fileio for reading ELF files")
    target_sources(mwr PRIVATE ${src}/mwr/utils/elf_nolibelf.cpp)
endif()

if(MWR_COVERAGE)
    target_compile_options(mwr PUBLIC --coverage)
    target_link_libraries(mwr PUBLIC -lgcov)
endif()

if(MWR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

install(TARGETS mwr DESTINATION lib)
install(DIRECTORY ${inc}/ DESTINATION include)
install(DIRECTORY ${gen}/ DESTINATION include)
