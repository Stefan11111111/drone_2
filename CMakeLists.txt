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

add_executable(verification_workflow_test tests/verification_workflow_test.cpp)
target_link_libraries(verification_workflow_test PRIVATE GTest::gtest_main)
gtest_discover_tests(verification_workflow_test)

add_executable(domain_participant_smoke_test tests/domain_participant_smoke_test.cpp)
target_link_libraries(domain_participant_smoke_test PRIVATE GTest::gtest_main fastdds)
gtest_discover_tests(domain_participant_smoke_test)

add_executable(domain_values_test tests/domain_values_test.cpp)
target_link_libraries(domain_values_test PRIVATE GTest::gtest_main drone_domain)
gtest_discover_tests(domain_values_test)

add_executable(target_track_test tests/target_track_test.cpp)
target_link_libraries(target_track_test PRIVATE GTest::gtest_main drone_domain)
gtest_discover_tests(target_track_test)

add_executable(drone_state_test tests/drone_state_test.cpp)
target_link_libraries(drone_state_test PRIVATE GTest::gtest_main drone_domain)
gtest_discover_tests(drone_state_test)

add_executable(explosion_event_test tests/explosion_event_test.cpp)
target_link_libraries(explosion_event_test PRIVATE GTest::gtest_main drone_domain)
gtest_discover_tests(explosion_event_test)

add_executable(control_messages_test tests/control_messages_test.cpp)
target_link_libraries(control_messages_test PRIVATE GTest::gtest_main drone_domain)
gtest_discover_tests(control_messages_test)
