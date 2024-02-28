 ##############################################################################
 #                                                                            #
 # Copyright (C) 2022 MachineWare GmbH                                        #
 # All Rights Reserved                                                        #
 #                                                                            #
 # This is work is licensed under the terms described in the LICENSE file     #
 # found in the root directory of this source tree.                           #
 #                                                                            #
 ##############################################################################

macro(util_test test)
    add_executable(${test} ${test}.cpp)
    target_include_directories(${test} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_link_libraries(${test} mwr gtest gmock)
    target_compile_options(${test} PRIVATE ${MWR_COMPILER_WARN_FLAGS})
    set_target_properties(${test} PROPERTIES CXX_CLANG_TIDY "${MWR_LINTER}")
    add_test(NAME utils/${test} COMMAND ${test} ${resources})
    set_tests_properties(utils/${test} PROPERTIES ENVIRONMENT "${MWR_ENVVAR}")
    set_tests_properties(utils/${test} PROPERTIES TIMEOUT 4)
endmacro()

util_test(aio)
util_test(elf)
util_test(license)
util_test(library)
util_test(locale)
util_test(memory)
util_test(modules)
util_test(options)
util_test(socket)
util_test(srec)
util_test(terminal)
util_test(uimage)
