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
  )

set(srs_converter_SRC
  ${ClusteringBase_SRC}
  old_formats/srs_converter.cpp)
set(srs_converter_INC
  old_formats/OldReadoutFile.h
  ${ClusteringBase_INC})
#create_executable(srs_converter)

set(NMXRefDataValidation_SRC
  ${ClusteringBase_SRC}
  RefDataValidationTest.cpp)
set(NMXRefDataValidation_INC
  ${ClusteringBase_INC})
create_test_executable(NMXRefDataValidation SKIP_MEMGRIND)
target_compile_definitions(NMXRefDataValidation
  PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}")

set(HitSorterTest_SRC
  ${ClusteringBase_SRC}
  HitSorterTest.cpp)
set(HitSorterTest_INC
  ${ClusteringBase_INC})
create_test_executable(HitSorterTest SKIP_MEMGRIND)
target_compile_definitions(HitSorterTest
  PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}")

set(NMXRefDataClusteringTest_SRC
  ${ClusteringBase_SRC}
  ../../common/clustering/GapClusterer.cpp
  RefDataClusteringTest.cpp)
set(NMXRefDataClusteringTest_INC
  ../../common/clustering/GapClusterer.h
  ${ClusteringBase_INC})
create_test_executable(NMXRefDataClusteringTest SKIP_MEMGRIND)
target_compile_definitions(NMXRefDataClusteringTest
  PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}")

set(NMXRefDataMatchingTest_SRC
  ${ClusteringBase_SRC}
  ../../common/clustering/GapMatcher.cpp
  ../../common/clustering/GapClusterer.cpp
  RefDataMatchingTest.cpp)
set(NMXRefDataMatchingTest_INC
  ../../common/clustering/GapMatcher.h
  ../../common/clustering/GapClusterer.h
  ${ClusteringBase_INC})
create_test_executable(NMXRefDataMatchingTest SKIP_MEMGRIND)
target_compile_definitions(NMXRefDataMatchingTest
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
