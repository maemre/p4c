# Copyright 2013-present Barefoot Networks, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Makefile for the tc P4-16 back-end.
# To be included in the main P4C compiler makefile

message(STATUS "Start configuring tc back end")

set (P4C_TC_SRCS
  dead_state_elimination.cpp
  inlining.cpp
  instruction.cpp
  ir_builder.cpp
  p4c_tc.cpp
  pass_manager.cpp
  reachability.cpp
  simulator.cpp
  tcam_program.cpp
  test_util.cpp
  util.cpp
  yaml_parser.cpp
  yaml_serializer.cpp
  )

set (P4C_TC_HDRS
  dead_state_elimination.h
  inlining.h
  instruction.h
  ir_builder.h
  p4c_interface.h
  pass.h
  pass_manager.h
  reachability.h
  simulator.h
  tcam_program.h
  test_util.h
  util.h
  yaml_parser.h
  yaml_serializer.h
  )

add_cpplint_files(${CMAKE_CURRENT_SOURCE_DIR} "${P4C_TC_SRCS};${P4C_TC_HDRS}")

# check for abseil libraries
find_library(ABSEIL_BASE NAMES absl_base)
if (ABSEIL_BASE)
  message(STATUS "Found abseil_base library")
else()
  message(FATAL_ERROR "Missing abseil_base dependency, cannot build the tc backend.")
endif()
find_library(ABSEIL_CITY NAMES absl_city)
if (ABSEIL_CITY)
  message(STATUS "Found abseil_city library")
else()
  message(FATAL_ERROR "Missing abseil_city dependency, cannot build the tc backend.")
endif()
find_library(ABSEIL_FLAGS NAMES absl_flags)
if (ABSEIL_FLAGS)
  message(STATUS "Found abseil_flags library")
else()
  message(FATAL_ERROR "Missing abseil_flags dependency, cannot build the tc backend.")
endif()
find_library(ABSEIL_STATUS NAMES absl_status)
if (ABSEIL_STATUS)
  message(STATUS "Found abseil_status library")
else()
  message(FATAL_ERROR "Missing abseil_status dependency, cannot build the tc backend.")
endif()
find_library(ABSEIL_STATUSOR NAMES absl_statusor)
if (ABSEIL_STATUSOR)
  message(STATUS "Found abseil_statusor library")
else()
  message(FATAL_ERROR "Missing abseil_statusor dependency, cannot build the tc backend.")
endif()
find_library(ABSEIL_HASH NAMES absl_hash)
if (ABSEIL_HASH)
  message(STATUS "Found abseil_hash library")
else()
  message(FATAL_ERROR "Missing abseil_hash dependency, cannot build the tc backend.")
endif()
find_library(ABSEIL_STRINGS NAMES absl_strings)
if (ABSEIL_STRINGS)
  message(STATUS "Found abseil_strings library")
else()
  message(FATAL_ERROR "Missing abseil_strings dependency, cannot build the tc backend.")
endif()
find_library(ABSEIL_STRINGS_INTERNAL NAMES absl_strings_internal)
if (ABSEIL_STRINGS_INTERNAL)
  message(STATUS "Found abseil_strings library")
else()
  message(FATAL_ERROR "Missing abseil_strings dependency, cannot build the tc backend.")
endif()
find_library(ABSEIL_THROW_DELEGATE NAMES absl_throw_delegate)
if (ABSEIL_THROW_DELEGATE)
  message(STATUS "Found abseil_throw_delegate library")
else()
  message(FATAL_ERROR "Missing abseil_throw_delegate dependency, cannot build the tc backend.")
endif()

set (ABSEIL_LIBRARIES
  ${ABSEIL_THROW_DELEGATE}
  ${ABSEIL_BASE}
  ${ABSEIL_CITY}
  ${ABSEIL_FLAGS}
  ${ABSEIL_HASH}
  ${ABSEIL_STATUS}
  ${ABSEIL_STATUSOR}
  ${ABSEIL_STRINGS}
  ${ABSEIL_STRINGS_INTERNAL}
  )

# check for yaml-cpp
find_library(YAMLCPP NAMES yaml-cpp yaml-cpp-0.7.0 yamlcpp)
if (YAMLCPP)
  message(STATUS "Found yaml-cpp library")
else()
  message(FATAL_ERROR "Missing yaml-cpp dependency, cannot build the tc backend.")
endif()

build_unified(P4C_TC_SRCS)
add_executable(p4c-tc ${P4C_TC_SRCS})
target_link_libraries (p4c-tc ${P4C_LIBRARIES} ${P4C_LIB_DEPS} ${ABSEIL_LIBRARIES} ${YAMLCPP})
add_dependencies(p4c-tc genIR frontend)

install (TARGETS p4c-tc
  RUNTIME DESTINATION ${P4C_RUNTIME_OUTPUT_DIRECTORY})
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/p4include
  DESTINATION ${P4C_ARTIFACTS_OUTPUT_DIRECTORY})

file(RELATIVE_PATH
  CURRENT_BINARY_DIR_PATH_REL
  ${P4C_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

file(RELATIVE_PATH
  P4C_BINARY_DIR_PATH_REL
  ${CMAKE_CURRENT_BINARY_DIR}
  ${P4C_BINARY_DIR}
)

# hack to get around the fact that the test scripts expect the backend
# binary to be in the top level directory. This should go away when we
# remove automake and fix the scripts.
add_custom_target(linkp4ctc
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${CURRENT_BINARY_DIR_PATH_REL}/p4c-tc ${P4C_BINARY_DIR}/p4c-tc
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${P4C_BINARY_DIR_PATH_REL}/p4include ${CMAKE_CURRENT_BINARY_DIR}/p4include
  )

add_dependencies(p4c_driver linkp4ctc)

# Tests

# TODO: Set up tests using GoogleTest

message(STATUS "Done with configuring tc back end")
