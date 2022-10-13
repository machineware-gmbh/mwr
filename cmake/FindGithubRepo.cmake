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

function(CloneGithubRepo target repo)
    string(TOUPPER ${target} _pfx)
    string(REPLACE "-" "_" _pfx ${_pfx})

    set(${_pfx}_REPO "${repo}" CACHE STRING "${target} repository url")
    set(${_pfx}_TAG "default" CACHE STRING "${target} repository tag/branch")

    find_package(Git REQUIRED)
    set(cmd clone --depth 1)

    if(DEFINED ENV{GITHUB_TOKEN})
        set(url "https://oauth2:$ENV{GITHUB_TOKEN}@github.com/${${_pfx}_REPO}")
    else()
        set(url "git@github.com:${${_pfx}_REPO}")
    endif()

    if(NOT ${_pfx}_TAG STREQUAL "default")
        set(cmd ${cmd} --branch ${${_pfx}_TAG})
    endif()

    message(STATUS "Fetching ${target} from ${${_pfx}_REPO} [${${_pfx}_TAG}]")
    execute_process(COMMAND ${GIT_EXECUTABLE} ${cmd} ${url} ${${_pfx}_HOME}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    ERROR_VARIABLE err RESULT_VARIABLE res OUTPUT_QUIET)
    if(res)
        message(FATAL_ERROR "${err}")
    endif()
endfunction()


macro(FindGithubRepo target repo)
    string(TOUPPER ${target} _pfx)
    string(REPLACE "-" "_" _pfx ${_pfx})

    if(NOT TARGET ${target})
        if(NOT DEFINED ${_pfx}_HOME AND DEFINED ENV{${_pfx}_HOME})
            set(${_pfx}_HOME $ENV{${_pfx}_HOME})
        endif()

        if(NOT DEFINED ${_pfx}_HOME)
            set(${_pfx}_HOME "${CMAKE_CURRENT_BINARY_DIR}/${target}")
        endif()

        if(NOT EXISTS ${${_pfx}_HOME}/CMakeLists.txt)
            CloneGithubRepo(${target} ${repo})
        endif()

        if(EXISTS ${${_pfx}_HOME}/CMakeLists.txt)
            add_subdirectory(${${_pfx}_HOME} ${target} EXCLUDE_FROM_ALL)
        endif()
    endif()

    if(NOT TARGET ${target})
        message(FATAL_ERROR "Cannot find ${target}")
    endif()

    set(${_pfx}_LIBRARIES ${target})
    get_target_property(type ${target} TYPE)
    if ("${type}" STREQUAL "INTERFACE_LIBRARY")
        if (CMAKE_VERSION GREATER_EQUAL "3.19")
            get_target_property(${_pfx}_HOME ${target} SOURCE_DIR)
        elseif(NOT ${_pfx}_HOME)
            set(${_pfx}_HOME "${CMAKE_CURRENT_BINARY_DIR}/${target}")
        endif()
        get_target_property(${_pfx}_INCLUDE_DIRS ${target} INTERFACE_INCLUDE_DIRECTORIES)
        if (CMAKE_VERSION GREATER_EQUAL "3.19")
            get_target_property(${_pfx}_VERSION ${target} VERSION)
        endif()
    else()
        get_target_property(${_pfx}_HOME ${target} SOURCE_DIR)
        get_target_property(${_pfx}_INCLUDE_DIRS ${target} INCLUDE_DIRECTORIES)
        get_target_property(${_pfx}_VERSION ${target} VERSION)
    endif()

    if(NOT ${_pfx}_VERSION)
        set(${_pfx}_VERSION "0.0.0")
    endif()

    include(FindPackageHandleStandardArgs)
    if(CMAKE_VERSION GREATER_EQUAL "3.17")
        set(_mismatch NAME_MISMATCHED)
    endif()

    find_package_handle_standard_args(${_pfx}
        REQUIRED_VARS ${_pfx}_LIBRARIES ${_pfx}_INCLUDE_DIRS
        VERSION_VAR   ${_pfx}_VERSION
        ${_mismatch})
    mark_as_advanced(${_pfx}_LIBRARIES ${_pfx}_INCLUDE_DIRS)

    message(DEBUG "${_pfx}_FOUND         " ${${_pfx}_FOUND})
    message(DEBUG "${_pfx}_HOME          " ${${_pfx}_HOME})
    message(DEBUG "${_pfx}_INCLUDE_DIRS  " ${${_pfx}_INCLUDE_DIRS})
    message(DEBUG "${_pfx}_LIBRARIES     " ${${_pfx}_LIBRARIES})
    message(DEBUG "${_pfx}_VERSION       " ${${_pfx}_VERSION})
endmacro()
