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

macro(core_test test)
    add_executable(${test} ${test}.cpp)
    target_include_directories(${test} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_link_libraries(${test} mwr gtest gmock)
    target_compile_features(${test} PRIVATE cxx_std_17)
    set_target_properties(${test} PROPERTIES CXX_CLANG_TIDY "${MWR_LINTER}")
    add_test(NAME core/${test} COMMAND ${test} ${resources})
    set_tests_properties(core/${test} PROPERTIES ENVIRONMENT "${MWR_ENVVAR}")
    set_tests_properties(core/${test} PROPERTIES TIMEOUT 4)
endmacro()

core_test(atomics)
core_test(bitops)
core_test(bitfields)
core_test(muldiv)
core_test(compiler)
core_test(types)
core_test(report)
core_test(utils)
core_test(sfinae)
core_test(saturate)
