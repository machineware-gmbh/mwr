 ##############################################################################
 #                                                                            #
 # Copyright (C) 2023 MachineWare GmbH                                        #
 # All Rights Reserved                                                        #
 #                                                                            #
 # This is work is licensed under the terms described in the LICENSE file     #
 # found in the root directory of this source tree.                           #
 #                                                                            #
 ##############################################################################

macro(logging_test test)
    add_executable(${test} ${test}.cpp)
    target_include_directories(${test} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_link_libraries(${test} mwr gtest gmock)
    target_compile_options(${test} PRIVATE ${MWR_COMPILER_WARN_FLAGS})
    set_target_properties(${test} PROPERTIES CXX_CLANG_TIDY "${MWR_LINTER}")
    add_test(NAME logging/${test} COMMAND ${test} ${resources})
    set_tests_properties(logging/${test} PROPERTIES ENVIRONMENT "${MWR_ENVVAR}")
    set_tests_properties(logging/${test} PROPERTIES TIMEOUT 4)
endmacro()

logging_test(levels)
logging_test(publisher)
