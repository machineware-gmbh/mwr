 ##############################################################################
 #                                                                            #
 # Copyright (C) 2022 MachineWare GmbH                                        #
 # All Rights Reserved                                                        #
 #                                                                            #
 # This work is licensed under the terms described in the LICENSE file found  #
 # in the root directory of this source tree.                                 #
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
    set_tests_properties(utils/${test} PROPERTIES TIMEOUT 30)
endmacro()

util_test(aio)
util_test(elf)
util_test(fdt)
util_test(hostinfo)
util_test(ihex)
util_test(interval)
util_test(library)
util_test(license)
util_test(locale)
util_test(memory)
util_test(modules)
util_test(options)
util_test(per_thread)
util_test(server_socket)
util_test(socket)
util_test(srec)
util_test(subprocess)
util_test(terminal)
util_test(uimage)
util_test(watchdog)

macro(hostinfo_test name)
    add_test(NAME utils/hostinfo-${name} COMMAND hostinfo ${resources})
    set_tests_properties(utils/hostinfo-${name} PROPERTIES ENVIRONMENT "${MWR_ENVVAR}")
    set_tests_properties(utils/hostinfo-${name} PROPERTIES TIMEOUT 30)
    foreach(feature ${ARGN})
        set_property(TEST utils/hostinfo-${name} APPEND PROPERTY
                     ENVIRONMENT "MWR_NO_${feature}=1")
    endforeach()
endmacro()

hostinfo_test(noavx AVX)
hostinfo_test(none BMI1 BMI2 CMOV MOVBE POPCNT LZCNT AVX AVX2 FMA F16C AVX512 SVE FP16 BF16)
