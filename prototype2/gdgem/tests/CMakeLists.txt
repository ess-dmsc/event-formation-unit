#=============================================================================
# Tests
#=============================================================================

set(TEST_DATA_PATH "${REFDATA}/gdgem/2018_11")

set(ClusteringBase_SRC
  ../NMXConfig.cpp
  ../../common/clustering/Hit.cpp
  ../srs/SRSTime.cpp
  ../srs/SRSMappings.cpp
  ../srs/CalibrationFile.cpp
  )
set(ClusteringBase_INC
  ../NMXConfig.h
  ../nmx/Readout.h
  ../../common/clustering/Hit.h
  ../nmx/Readout.h
  ../srs/SRSTime.h
  ../srs/SRSMappings.h
  ../srs/CalibrationFile.h
  )

set(NMXReferenceDataTest_SRC
  ${ClusteringBase_SRC}
  ../../common/clustering/GapClusterer.cpp
  ReferenceDataTest.cpp)
set(NMXReferenceDataTest_INC
  ../../common/clustering/GapClusterer.h
  ${ClusteringBase_INC})
create_test_executable(NMXReferenceDataTest SKIP_MEMGRIND)
target_compile_definitions(NMXReferenceDataTest
  PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}")

## GOOGLE BENCHMARK
message(STATUS "!!! *** Skipping NMXClustererBenchmarkTest - does not link ***")
# set(NMXClustererBenchmarkTest_SRC
#     ${ClusteringBase_SRC}
#     NMXClustererBenchmarkTest.cpp)
# set(NMXClustererBenchmarkTest_INC
#     ${ClusteringBase_INC})
# create_benchmark_executable(NMXClustererBenchmarkTest)
