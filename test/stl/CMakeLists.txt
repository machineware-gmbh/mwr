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

macro(stl_test test)
    add_executable(${test} ${test}.cpp)
    target_link_libraries(${test} mwr gtest gmock gtest_main)
    set_target_properties(${test} PROPERTIES CXX_CLANG_TIDY "${MWR_LINTER}")
    add_test(NAME stl/${test} COMMAND ${test})
    set_tests_properties(stl/${test} PROPERTIES ENVIRONMENT "${MWR_ENVVAR}")
    set_tests_properties(stl/${test} PROPERTIES TIMEOUT 4)
endmacro()

stl_test(strings)
stl_test(container)
