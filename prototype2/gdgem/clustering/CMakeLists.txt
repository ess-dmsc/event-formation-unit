#=============================================================================
# Tests
#=============================================================================

# Clustering

set(TEST_DATA_PATH "${REFDATA}/gdgem/2018_11")

set(ClusteringBase_SRC
  ../NMXConfig.cpp
  ../nmx/Cluster.cpp
  ../../common/clustering/Hit.cpp
  ../srs/SRSTime.cpp
  ../srs/SRSMappings.cpp
  ../vmm3/CalibrationFile.cpp
  HitSorter.cpp
  )
set(ClusteringBase_INC
  ../NMXConfig.h
  ../nmx/Readout.h
  ../nmx/Cluster.h
  ../../common/clustering/Hit.h
  ../nmx/Readout.h
  ../srs/SRSTime.h
  ../srs/SRSMappings.h
  ../vmm3/CalibrationFile.h
  HitSorter.h
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
  ../../common/clustering/GapClusterer.cpp
  DoroClustererTest.cpp)
set(DoroClustererTest_INC
  ../../common/clustering/GapClusterer.h
  ${ClusteringBase_INC})
create_test_executable(DoroClustererTest SKIP_MEMGRIND)
target_compile_definitions(DoroClustererTest
  PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}")

set(ClusterMatcherTest_SRC
  ${ClusteringBase_SRC}
  ../../common/clustering/GapMatcher.cpp
  ../../common/clustering/GapClusterer.cpp
  ClusterMatcherTest.cpp)
set(ClusterMatcherTest_INC
  ../../common/clustering/GapMatcher.h
  ../../common/clustering/GapClusterer.h
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
