cmake_minimum_required(VERSION 3.30...4.99)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 5)
  message(FATAL_ERROR "CMake 5 or newer is not supported; use CMake 3.30 through 4.x")
endif()

project(drone_interception LANGUAGES CXX)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(FATAL_ERROR "The supported compiler is GCC 15.2.x (configure with g++-15)")
endif()

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 15.2 OR
   NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 15.3)
  message(FATAL_ERROR "The supported compiler version is GCC 15.2.x")
endif()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_executable(cpp26_build_probe tests/cpp26_build_probe.cpp)
add_test(NAME cpp26_build_probe COMMAND cpp26_build_probe)

find_package(GTest 1.17...<1.18 CONFIG REQUIRED)
find_package(fastcdr 2.3.5 EXACT CONFIG REQUIRED)
find_package(fastdds 3.3.0 EXACT CONFIG REQUIRED)
include(GoogleTest)

find_program(FASTDDSGEN_EXECUTABLE NAMES fastddsgen REQUIRED)
execute_process(
  COMMAND ${FASTDDSGEN_EXECUTABLE} -version
  RESULT_VARIABLE fastddsgen_result
  OUTPUT_VARIABLE fastddsgen_version_output
  ERROR_VARIABLE fastddsgen_version_error
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE)
if(NOT fastddsgen_result EQUAL 0)
  message(
    FATAL_ERROR
      "Failed to query Fast DDS-Gen at ${FASTDDSGEN_EXECUTABLE}: ${fastddsgen_version_error}")
endif()
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" FASTDDSGEN_VERSION "${fastddsgen_version_output}")
if(NOT FASTDDSGEN_VERSION STREQUAL "4.2.0")
  message(
    FATAL_ERROR
      "Fast DDS-Gen 4.2.0 is required; ${FASTDDSGEN_EXECUTABLE} reported '${fastddsgen_version_output}'")
endif()

set(target_track_idl ${CMAKE_CURRENT_SOURCE_DIR}/src/drone_dds_types/idl/target_track.idl)
set(drone_dds_generated_dir ${CMAKE_CURRENT_BINARY_DIR}/generated/drone_dds_types)
set(
  target_track_generated_files
  ${drone_dds_generated_dir}/target_track.hpp
  ${drone_dds_generated_dir}/target_trackCdrAux.hpp
  ${drone_dds_generated_dir}/target_trackCdrAux.ipp
  ${drone_dds_generated_dir}/target_trackPubSubTypes.cxx
  ${drone_dds_generated_dir}/target_trackPubSubTypes.hpp
  ${drone_dds_generated_dir}/target_trackTypeObjectSupport.cxx
  ${drone_dds_generated_dir}/target_trackTypeObjectSupport.hpp)
add_custom_command(
  OUTPUT ${target_track_generated_files}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${drone_dds_generated_dir}
  COMMAND ${FASTDDSGEN_EXECUTABLE} -replace -d ${drone_dds_generated_dir} ${target_track_idl}
  DEPENDS ${target_track_idl}
  COMMENT "Generating drone DDS wire types from IDL"
  VERBATIM)

add_library(
  drone_dds_types
  ${drone_dds_generated_dir}/target_trackPubSubTypes.cxx
  ${drone_dds_generated_dir}/target_trackTypeObjectSupport.cxx)
target_include_directories(
  drone_dds_types SYSTEM PUBLIC $<BUILD_INTERFACE:${drone_dds_generated_dir}>)
target_link_libraries(drone_dds_types PUBLIC fastcdr fastdds)

add_library(
  drone_domain
  src/drone_domain/assignment.cpp
  src/drone_domain/drone_id.cpp
  src/drone_domain/drone_state.cpp
  src/drone_domain/explosion_event.cpp
  src/drone_domain/explosion_event_id.cpp
  src/drone_domain/interception_command.cpp
  src/drone_domain/interception_command_id.cpp
  src/drone_domain/position.cpp
  src/drone_domain/target_id.cpp
  src/drone_domain/target_track.cpp
  src/drone_domain/timestamp.cpp)
target_include_directories(drone_domain PUBLIC include)

add_library(observer_core src/observer_core/target_tracker.cpp)
target_include_directories(observer_core PUBLIC include)
target_link_libraries(observer_core PUBLIC drone_domain)

add_library(simulated_radar_adapter src/simulated_radar_adapter/simulated_radar.cpp)
target_include_directories(simulated_radar_adapter PUBLIC include)
target_link_libraries(simulated_radar_adapter PUBLIC observer_core)

add_library(
  drone_dds_transport
  src/drone_dds_transport/assignment_mapping.cpp
  src/drone_dds_transport/domain_participant_owner.cpp
  src/drone_dds_transport/drone_state_mapping.cpp
  src/drone_dds_transport/explosion_event_mapping.cpp
  src/drone_dds_transport/interception_command_mapping.cpp
  src/drone_dds_transport/target_track_reader.cpp
  src/drone_dds_transport/target_track_mapping.cpp
  src/drone_dds_transport/target_track_topic.cpp
  src/drone_dds_transport/target_track_writer.cpp)
target_include_directories(drone_dds_transport PUBLIC include)
target_link_libraries(drone_dds_transport PUBLIC drone_dds_types drone_domain fastdds)

add_executable(verification_workflow_test tests/verification_workflow_test.cpp)
target_link_libraries(verification_workflow_test PRIVATE GTest::gtest_main)
gtest_discover_tests(verification_workflow_test)

add_test(
  NAME VerificationWorkflow.TopicCatalog
  COMMAND
    ${CMAKE_COMMAND}
    -DCATALOG=${CMAKE_CURRENT_SOURCE_DIR}/docs/dds-topic-catalog.md
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/verify_topic_catalog.cmake)

add_executable(domain_participant_smoke_test tests/domain_participant_smoke_test.cpp)
target_link_libraries(domain_participant_smoke_test PRIVATE GTest::gtest_main fastdds)
gtest_discover_tests(domain_participant_smoke_test)

add_executable(domain_participant_owner_test tests/domain_participant_owner_test.cpp)
target_link_libraries(domain_participant_owner_test PRIVATE GTest::gtest_main drone_dds_transport)
gtest_discover_tests(domain_participant_owner_test)

add_executable(domain_values_test tests/domain_values_test.cpp)
target_link_libraries(domain_values_test PRIVATE GTest::gtest_main drone_domain)
gtest_discover_tests(domain_values_test)

add_executable(target_track_test tests/target_track_test.cpp)
target_link_libraries(target_track_test PRIVATE GTest::gtest_main drone_domain)
gtest_discover_tests(target_track_test)

add_executable(target_track_wire_type_test tests/target_track_wire_type_test.cpp)
target_link_libraries(target_track_wire_type_test PRIVATE GTest::gtest_main drone_dds_types)
gtest_discover_tests(target_track_wire_type_test)

add_executable(target_track_mapping_test tests/target_track_mapping_test.cpp)
target_link_libraries(target_track_mapping_test PRIVATE GTest::gtest_main drone_dds_transport)
gtest_discover_tests(target_track_mapping_test)

add_executable(target_track_pub_sub_test tests/target_track_pub_sub_test.cpp)
target_link_libraries(target_track_pub_sub_test PRIVATE GTest::gtest_main drone_dds_transport)
gtest_discover_tests(target_track_pub_sub_test)

add_executable(target_track_discovery_test tests/target_track_discovery_test.cpp)
target_link_libraries(target_track_discovery_test PRIVATE GTest::gtest_main drone_dds_transport)
gtest_discover_tests(target_track_discovery_test)

add_executable(drone_state_mapping_test tests/drone_state_mapping_test.cpp)
target_link_libraries(drone_state_mapping_test PRIVATE GTest::gtest_main drone_dds_transport)
gtest_discover_tests(drone_state_mapping_test)

add_executable(control_event_mapping_test tests/control_event_mapping_test.cpp)
target_link_libraries(control_event_mapping_test PRIVATE GTest::gtest_main drone_dds_transport)
gtest_discover_tests(control_event_mapping_test)

add_executable(drone_state_wire_type_test tests/drone_state_wire_type_test.cpp)
target_link_libraries(drone_state_wire_type_test PRIVATE GTest::gtest_main drone_dds_types)
gtest_discover_tests(drone_state_wire_type_test)

add_executable(control_wire_types_test tests/control_wire_types_test.cpp)
target_link_libraries(control_wire_types_test PRIVATE GTest::gtest_main drone_dds_types)
gtest_discover_tests(control_wire_types_test)

add_executable(explosion_event_wire_type_test tests/explosion_event_wire_type_test.cpp)
target_link_libraries(explosion_event_wire_type_test PRIVATE GTest::gtest_main drone_dds_types)
gtest_discover_tests(explosion_event_wire_type_test)

add_executable(drone_state_test tests/drone_state_test.cpp)
target_link_libraries(drone_state_test PRIVATE GTest::gtest_main drone_domain)
gtest_discover_tests(drone_state_test)

add_executable(explosion_event_test tests/explosion_event_test.cpp)
target_link_libraries(explosion_event_test PRIVATE GTest::gtest_main drone_domain)
gtest_discover_tests(explosion_event_test)

add_executable(control_messages_test tests/control_messages_test.cpp)
target_link_libraries(control_messages_test PRIVATE GTest::gtest_main drone_domain)
gtest_discover_tests(control_messages_test)

add_executable(observer_core_test tests/observer_core_test.cpp)
target_link_libraries(observer_core_test PRIVATE GTest::gtest_main observer_core)
gtest_discover_tests(observer_core_test)

add_executable(simulated_radar_adapter_test tests/simulated_radar_adapter_test.cpp)
target_link_libraries(
  simulated_radar_adapter_test PRIVATE GTest::gtest_main simulated_radar_adapter)
gtest_discover_tests(simulated_radar_adapter_test)
