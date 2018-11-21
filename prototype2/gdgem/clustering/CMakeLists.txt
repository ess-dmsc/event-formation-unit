#=============================================================================
# Tests
#=============================================================================

# Clustering

set(TEST_DATA_PATH "${REFDATA}/gdgem/2018_11")

set(ClusteringBase_SRC
  ../NMXConfig.cpp
  ../nmx/Event.cpp
  ../nmx/Cluster.cpp
  ../nmx/Hit.cpp
  ../srs/SRSTime.cpp
  ../srs/SRSMappings.cpp
  ../vmm3/CalibrationFile.cpp
  HitsQueue.cpp
  HitSorter.cpp
  ClusterMatcher.cpp
  )
set(ClusteringBase_INC
  ../NMXConfig.h
  ../nmx/Event.h
  ../nmx/Readout.h
  ../nmx/Cluster.h
  ../nmx/Hit.h
  ../nmx/Readout.h
  ../srs/SRSTime.h
  ../srs/SRSMappings.h
  ../vmm3/CalibrationFile.h
  HitContainer.h
  HitsQueue.h
  HitSorter.h
  AbstractClusterer.h
  ClusterMatcher.h
  TestDataShort.h
  )

set(srs_converter_SRC
  ${ClusteringBase_SRC}
  old_formats/srs_converter.cpp)
set(srs_converter_INC
  old_formats/OldReadoutFile.h
  ${ClusteringBase_INC})
#create_executable(srs_converter)

set(HitsQueueTest_SRC
  ${ClusteringBase_SRC}
  HitsQueueTest.cpp)
set(HitsQueueTest_INC
  ${ClusteringBase_INC})
create_test_executable(HitsQueueTest SKIP_MEMGRIND)
target_compile_definitions(HitsQueueTest
  PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}")

set(HitSorterTest_SRC
  ${ClusteringBase_SRC}
  HitSorterTest.cpp)
set(HitSorterTest_INC
  ${ClusteringBase_INC})
create_test_executable(HitSorterTest SKIP_MEMGRIND)
target_compile_definitions(HitSorterTest
  PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}")

set(DoroClustererTest_SRC
  ${ClusteringBase_SRC}
  DoroClusterer.cpp
  DoroClustererTest.cpp)
set(DoroClustererTest_INC
  DoroClusterer.h
  ${ClusteringBase_INC})
create_test_executable(DoroClustererTest SKIP_MEMGRIND)
target_compile_definitions(DoroClustererTest
  PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}")

set(ClusterMatcherTest_SRC
  ${ClusteringBase_SRC}
  ClusterMatcher.cpp
  DoroClusterer.cpp
  ClusterMatcherTest.cpp)
set(ClusterMatcherTest_INC
  ClusterMatcher.h
  DoroClusterer.h
  ${ClusteringBase_INC})
create_test_executable(ClusterMatcherTest SKIP_MEMGRIND)
target_compile_definitions(ClusterMatcherTest
  PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}")

## GOOGLE BENCHMARK
message(STATUS "!!! *** Skipping NMXClustererBenchmarkTest - does not link ***")
# set(NMXClustererBenchmarkTest_SRC
#     ${ClusteringBase_SRC}
#     NMXClustererBenchmarkTest.cpp)
# set(NMXClustererBenchmarkTest_INC
#     ${ClusteringBase_INC})
# create_benchmark_executable(NMXClustererBenchmarkTest)

set(GrayCodeBenchmarkTest_INC ../../../libs/include/BitMath.h)
set(GrayCodeBenchmarkTest_SRC GrayCodeBenchmarkTest.cpp)
create_benchmark_executable(GrayCodeBenchmarkTest)
