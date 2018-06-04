#=============================================================================
# Tests
#=============================================================================

# Clustering

get_filename_component(TEST_DATA_PATH "test_data/run16long.h5" DIRECTORY)

set(ClusteringBase_SRC
    ../nmx/Event.cpp
    ../nmx/Cluster.cpp
    ../nmx/Hit.cpp
    ../nmx/ReadoutFile.cpp
    ../srs/SRSTime.cpp
    ../srs/SRSMappings.cpp
    HitsQueue.cpp
    HitSorter.cpp
    ClusterMatcher.cpp
    )
set(ClusteringBase_INC
    ../nmx/Event.h
    ../nmx/Readout.h
    ../nmx/Cluster.h
    ../nmx/Hit.h
    ../nmx/ReadoutFile.h
    ../srs/SRSTime.h
    ../srs/SRSMappings.h
    HitContainer.h
    HitsQueue.h
    HitSorter.h
    AbstractClusterer.h
    ClusterMatcher.h
    TestDataShort.h
    )

#set(hdr2h5_SRC
#    ${ClusteringBase_SRC}
#    test_data/hdr2h5.cpp)
#set(hdr2h5_INC
#    ${ClusteringBase_INC})
#create_executable(hdr2h5)

set(HitsQueueTest_SRC
    ${ClusteringBase_SRC}
    HitsQueueTest.cpp)
set(HitsQueueTest_INC
    ${ClusteringBase_INC})
create_test_executable(HitsQueueTest SKIP_MEMGRIND)
target_compile_definitions(HitsQueueTest
    PRIVATE TEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DATA_PATH}/")

set(HitSorterTest_SRC
    ${ClusteringBase_SRC}
    HitSorterTest.cpp)
set(HitSorterTest_INC
    ${ClusteringBase_INC})
create_test_executable(HitSorterTest SKIP_MEMGRIND)
target_compile_definitions(HitSorterTest
    PRIVATE TEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DATA_PATH}/")

set(DoroClustererTest_SRC
    ${ClusteringBase_SRC}
    DoroClusterer.cpp
    DoroClustererTest.cpp)
set(DoroClustererTest_INC
    DoroClusterer.h
    ${ClusteringBase_INC})
create_test_executable(DoroClustererTest SKIP_MEMGRIND)
target_compile_definitions(DoroClustererTest
    PRIVATE TEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DATA_PATH}/")

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
    PRIVATE TEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DATA_PATH}/")

## GOOGLE BENCHMARK
set(NMXClustererBenchmarkTest_SRC
    ${ClusteringBase_SRC}
    NMXClustererBenchmarkTest.cpp)
set(NMXClustererBenchmarkTest_INC
    ${ClusteringBase_INC})
create_benchmark_executable(NMXClustererBenchmarkTest)

set(GrayCodeBenchmarkTest_INC ../../libs/include/BitMath.h)
set(GrayCodeBenchmarkTest_SRC GrayCodeBenchmarkTest.cpp)
create_benchmark_executable(GrayCodeBenchmarkTest)
