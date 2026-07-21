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

add_library(interceptor_core src/interceptor_core/interceptor_state_machine.cpp)
target_include_directories(interceptor_core PUBLIC include)
target_link_libraries(interceptor_core PUBLIC drone_domain)

add_library(
  console_core
  src/console_core/assignment_use_case.cpp
  src/console_core/drone_projection.cpp
  src/console_core/target_projection.cpp)
target_include_directories(console_core PUBLIC include)
target_link_libraries(console_core PUBLIC drone_domain)

add_library(console_ui_adapter src/console_ui_adapter/terminal_view.cpp)
target_include_directories(console_ui_adapter PUBLIC include)
target_link_libraries(console_ui_adapter PUBLIC console_core)

add_library(simulated_radar_adapter src/simulated_radar_adapter/simulated_radar.cpp)
target_include_directories(simulated_radar_adapter PUBLIC include)
target_link_libraries(simulated_radar_adapter PUBLIC observer_core)

add_library(simulated_vehicle_adapter src/simulated_vehicle_adapter/simulated_vehicle.cpp)
target_include_directories(simulated_vehicle_adapter PUBLIC include)
target_link_libraries(simulated_vehicle_adapter PUBLIC interceptor_core)

add_library(
  drone_dds_transport
  src/drone_dds_transport/assignment_mapping.cpp
  src/drone_dds_transport/domain_participant_owner.cpp
  src/drone_dds_transport/drone_state_mapping.cpp
  src/drone_dds_transport/drone_state_reader.cpp
  src/drone_dds_transport/drone_state_topic.cpp
  src/drone_dds_transport/drone_state_writer.cpp
  src/drone_dds_transport/explosion_event_mapping.cpp
  src/drone_dds_transport/interception_command_mapping.cpp
  src/drone_dds_transport/target_track_reader.cpp
  src/drone_dds_transport/target_track_mapping.cpp
  src/drone_dds_transport/target_track_topic.cpp
  src/drone_dds_transport/target_track_writer.cpp)
target_include_directories(drone_dds_transport PUBLIC include)
target_link_libraries(drone_dds_transport PUBLIC drone_dds_types drone_domain fastdds)

add_library(observer_dds_adapter src/observer_dds_adapter/target_track_publisher.cpp)
target_include_directories(observer_dds_adapter PUBLIC include)
target_link_libraries(observer_dds_adapter PUBLIC observer_core drone_dds_transport)

add_library(interceptor_dds_adapter src/interceptor_dds_adapter/drone_state_publisher.cpp)
target_include_directories(interceptor_dds_adapter PUBLIC include)
target_link_libraries(interceptor_dds_adapter PUBLIC interceptor_core drone_dds_transport)

add_library(console_dds_adapter src/console_dds_adapter/console_subscriber.cpp)
target_include_directories(console_dds_adapter PUBLIC include)
target_link_libraries(console_dds_adapter PUBLIC console_core drone_dds_transport)

add_executable(observer src/observer_app/main.cpp)
target_link_libraries(
  observer PRIVATE observer_core observer_dds_adapter simulated_radar_adapter)

add_executable(interceptor src/interceptor_app/main.cpp)
target_link_libraries(
  interceptor PRIVATE interceptor_core interceptor_dds_adapter simulated_vehicle_adapter)

add_executable(console src/console_app/main.cpp)
target_link_libraries(
  console PRIVATE console_core console_dds_adapter console_ui_adapter)

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

add_executable(interceptor_core_test tests/interceptor_core_test.cpp)
target_link_libraries(interceptor_core_test PRIVATE GTest::gtest_main interceptor_core)
gtest_discover_tests(interceptor_core_test)

add_executable(console_core_test tests/console_core_test.cpp)
target_link_libraries(console_core_test PRIVATE GTest::gtest_main console_core)
gtest_discover_tests(console_core_test)

add_executable(console_ui_adapter_test tests/console_ui_adapter_test.cpp)
target_link_libraries(
  console_ui_adapter_test PRIVATE GTest::gtest_main console_ui_adapter)
gtest_discover_tests(console_ui_adapter_test)

add_executable(console_dds_adapter_test tests/console_dds_adapter_test.cpp)
target_link_libraries(
  console_dds_adapter_test
  PRIVATE GTest::gtest_main console_dds_adapter drone_dds_transport drone_dds_types fastdds)
gtest_discover_tests(console_dds_adapter_test)

add_executable(simulated_radar_adapter_test tests/simulated_radar_adapter_test.cpp)
target_link_libraries(
  simulated_radar_adapter_test PRIVATE GTest::gtest_main simulated_radar_adapter)
gtest_discover_tests(simulated_radar_adapter_test)

add_executable(simulated_vehicle_adapter_test tests/simulated_vehicle_adapter_test.cpp)
target_link_libraries(
  simulated_vehicle_adapter_test PRIVATE GTest::gtest_main simulated_vehicle_adapter)
gtest_discover_tests(simulated_vehicle_adapter_test)

add_library(process_test_support tests/process_test_support.cpp)
target_include_directories(process_test_support PUBLIC tests)

add_executable(observer_publisher_process_test tests/observer_publisher_process_test.cpp)
target_compile_definitions(
  observer_publisher_process_test PRIVATE OBSERVER_EXECUTABLE_PATH="$<TARGET_FILE:observer>")
target_link_libraries(
  observer_publisher_process_test PRIVATE GTest::gtest_main drone_dds_transport process_test_support)
add_dependencies(observer_publisher_process_test observer)
gtest_discover_tests(observer_publisher_process_test)

add_executable(observer_console_process_test tests/observer_console_process_test.cpp)
target_compile_definitions(
  observer_console_process_test
  PRIVATE OBSERVER_EXECUTABLE_PATH="$<TARGET_FILE:observer>"
          CONSOLE_EXECUTABLE_PATH="$<TARGET_FILE:console>")
target_link_libraries(
  observer_console_process_test PRIVATE GTest::gtest_main process_test_support)
add_dependencies(observer_console_process_test observer console)
gtest_discover_tests(observer_console_process_test PROPERTIES TIMEOUT 15)

add_executable(live_system_state_process_test tests/live_system_state_process_test.cpp)
target_compile_definitions(
  live_system_state_process_test
  PRIVATE OBSERVER_EXECUTABLE_PATH="$<TARGET_FILE:observer>"
          CONSOLE_EXECUTABLE_PATH="$<TARGET_FILE:console>"
          INTERCEPTOR_EXECUTABLE_PATH="$<TARGET_FILE:interceptor>")
target_link_libraries(
  live_system_state_process_test PRIVATE GTest::gtest_main process_test_support)
add_dependencies(live_system_state_process_test observer console interceptor)
gtest_discover_tests(live_system_state_process_test PROPERTIES TIMEOUT 15)

add_executable(late_joiner_drone_state_process_test tests/late_joiner_drone_state_process_test.cpp)
target_compile_definitions(
  late_joiner_drone_state_process_test
  PRIVATE CONSOLE_EXECUTABLE_PATH="$<TARGET_FILE:console>"
          INTERCEPTOR_EXECUTABLE_PATH="$<TARGET_FILE:interceptor>")
target_link_libraries(
  late_joiner_drone_state_process_test PRIVATE GTest::gtest_main process_test_support)
add_dependencies(late_joiner_drone_state_process_test console interceptor)
gtest_discover_tests(late_joiner_drone_state_process_test PROPERTIES TIMEOUT 20)

add_executable(interceptor_publisher_process_test tests/interceptor_publisher_process_test.cpp)
target_compile_definitions(
  interceptor_publisher_process_test
  PRIVATE INTERCEPTOR_EXECUTABLE_PATH="$<TARGET_FILE:interceptor>")
target_link_libraries(
  interceptor_publisher_process_test
  PRIVATE GTest::gtest_main drone_dds_transport drone_dds_types fastdds process_test_support)
add_dependencies(interceptor_publisher_process_test interceptor)
gtest_discover_tests(interceptor_publisher_process_test PROPERTIES TIMEOUT 15)
