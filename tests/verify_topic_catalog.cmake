if(NOT DEFINED CATALOG)
  message(FATAL_ERROR "CATALOG must name the DDS topic catalog")
endif()

if(NOT EXISTS "${CATALOG}")
  message(FATAL_ERROR "DDS topic catalog not found: ${CATALOG}")
endif()

file(READ "${CATALOG}" catalog_contents)

set(
  expected_topics
  drone.target_tracks
  drone.drone_states
  drone.assignments
  drone.interception_commands
  drone.explosion_events)

foreach(topic IN LISTS expected_topics)
  string(REPLACE "." "\\." topic_pattern "${topic}")
  string(REGEX MATCHALL "`${topic_pattern}`" topic_occurrences "${catalog_contents}")
  list(LENGTH topic_occurrences occurrence_count)
  if(NOT occurrence_count EQUAL 1)
    message(
      FATAL_ERROR
        "Expected topic '${topic}' exactly once in the catalog, found ${occurrence_count}")
  endif()
endforeach()

string(
  REGEX MATCHALL
  "\\| `drone\\.[a-z_]+` \\|"
  catalog_topic_rows
  "${catalog_contents}")
list(LENGTH catalog_topic_rows topic_row_count)
list(LENGTH expected_topics expected_topic_count)
if(NOT topic_row_count EQUAL expected_topic_count)
  message(
    FATAL_ERROR
      "Expected ${expected_topic_count} catalog topic rows, found ${topic_row_count}")
endif()

set(
  required_contract_terms
  "drone::dds::TargetTrack"
  "drone::dds::DroneState"
  "drone::dds::Assignment"
  "drone::dds::InterceptionCommand"
  "drone::dds::ExplosionEvent"
  "RELIABLE"
  "TRANSIENT_LOCAL"
  "VOLATILE"
  "KEEP_LAST(1)"
  "ResourceLimitsQosPolicy"
  "Observer"
  "Console"
  "Interceptor")

foreach(term IN LISTS required_contract_terms)
  string(FIND "${catalog_contents}" "${term}" term_position)
  if(term_position EQUAL -1)
    message(FATAL_ERROR "DDS topic catalog is missing required contract term '${term}'")
  endif()
endforeach()

set(
  required_evidence_tests
  "QosCatalogAudit.GivenFinalTopicCatalog_WhenEndpointQosIsBuilt_ThenEveryWriterAndReaderMatchesIt"
  "TargetTrackPublishSubscribe.GivenTwoParticipants_WhenOneTrackIsWrittenAndTaken_ThenTheDomainValueRoundTrips"
  "TargetTrackDiscovery.GivenBestEffortWriterAndReliableReader_WhenEndpointsAreDiscovered_ThenBothReportReliabilityIncompatibility"
  "QosExperiments.GivenTransientLocalKeepLastHistory_WhenAReaderJoinsLate_ThenOnlyTheLatestSamplePerKeyArrives"
  "QosExperiments.GivenVolatileAssignmentHistory_WhenAReaderJoinsLate_ThenOldIntentIsNotReplayedButNewIntentArrives"
  "QosExperiments.GivenVolatileWriterAndTransientLocalReader_WhenDiscovered_ThenBothReportDurabilityIncompatibility"
  "QosExperiments.GivenTargetWriterAtCatalogCapacity_WhenANewKeyIsWritten_ThenTheResourceErrorIsVisible")

foreach(test_name IN LISTS required_evidence_tests)
  string(FIND "${catalog_contents}" "${test_name}" test_name_position)
  if(test_name_position EQUAL -1)
    message(FATAL_ERROR "DDS topic catalog is missing QoS evidence test '${test_name}'")
  endif()
endforeach()
