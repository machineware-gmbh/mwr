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

cmake_minimum_required(VERSION 3.11)
project(mwr VERSION 2023.02.09 LANGUAGES CXX)

set(MWR_LINTER "" CACHE STRING "Code linter to use")
set(MWR_BUILD_TESTS ON CACHE BOOL "Build unit tests")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(GenVersionInfo)
include(Sanitizer)

find_package(Threads REQUIRED)
find_package(LibELF REQUIRED)

set(src ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(inc ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(gen ${CMAKE_CURRENT_BINARY_DIR}/gen)

configure_file(${src}/mwr/core/version.h.in
               ${gen}/mwr/core/version.h @ONLY)

add_library(mwr STATIC
            ${src}/mwr/core/utils.cpp
            ${src}/mwr/core/report.cpp
            ${src}/mwr/core/bitops.cpp
            ${src}/mwr/core/terminal.cpp
            ${src}/mwr/stl/strings.cpp
            ${src}/mwr/stl/threads.cpp
            ${src}/mwr/utils/aio.cpp
            ${src}/mwr/utils/elf.cpp
            ${src}/mwr/utils/library.cpp
            ${src}/mwr/utils/license.cpp
            ${src}/mwr/utils/modules.cpp
            ${src}/mwr/utils/options.cpp
            ${src}/mwr/utils/socket.cpp
            ${src}/mwr/utils/srec.cpp)

target_include_directories(mwr PUBLIC ${inc})
target_include_directories(mwr PUBLIC ${gen})
target_include_directories(mwr SYSTEM PRIVATE ${LIBELF_INCLUDE_DIRS})
target_compile_options(mwr PRIVATE -Wall -Werror)
target_compile_features(mwr PRIVATE cxx_std_17)
target_compile_definitions(mwr PUBLIC $<$<CONFIG:DEBUG>:MWR_DEBUG>)
target_link_libraries(mwr PUBLIC -rdynamic)
target_link_libraries(mwr PUBLIC -lrt)
target_link_libraries(mwr PUBLIC -lstdc++fs)
target_link_libraries(mwr PUBLIC Threads::Threads)
target_link_libraries(mwr PUBLIC ${CMAKE_DL_LIBS})
target_link_libraries(mwr PUBLIC ${LIBELF_LIBRARIES})
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

