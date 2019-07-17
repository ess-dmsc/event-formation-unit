#=============================================================================
# Tests
#=============================================================================

set(TEST_DATA_PATH "${REFDATA}/gdgem/2018_11")

set(ClusteringBase_SRC
  ../NMXConfig.cpp
  ../srs/SRSTime.cpp
  ../srs/SRSMappings.cpp
  ../srs/CalibrationFile.cpp
  ../../multigrid/reduction/ModuleGeometry.cpp
  )
set(ClusteringBase_INC
  ../NMXConfig.h
  ../nmx/Readout.h
  ../srs/SRSTime.h
  ../srs/SRSMappings.h
  ../srs/CalibrationFile.h
  ../../multigrid/reduction/ModuleGeometry.h
  )

if(EXISTS ${TEST_DATA_PATH})

  set(NMXReferenceDataTest_SRC
    ${ClusteringBase_SRC}
    ReferenceDataTest.cpp)
  set(NMXReferenceDataTest_INC
    ${ClusteringBase_INC})
  create_test_executable(NMXReferenceDataTest SKIP_MEMGRIND)
  target_compile_definitions(NMXReferenceDataTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}")

  ## GOOGLE BENCHMARK
  set(NMXClustererBenchmarkTest_SRC
    ${ClusteringBase_SRC}
    ClustererBenchmarkTest.cpp
    )
  set(NMXClustererBenchmarkTest_INC
    ${ClusteringBase_INC})
  create_benchmark_executable(NMXClustererBenchmarkTest)
  if(GOOGLE_BENCHMARK)
    target_compile_definitions(NMXClustererBenchmarkTest
      PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}")
    target_link_libraries(NMXClustererBenchmarkTest
      GraylogLogger::graylog_logger_static
      fmt::fmt
      h5cpp
      )
  endif()

else()

  message(WARNING "*** No gdgem reference data found. Skipping relevant tests.")

endif()
