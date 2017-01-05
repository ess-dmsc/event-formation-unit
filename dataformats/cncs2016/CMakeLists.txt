cmake_minimum_required (VERSION 2.6)
set (CMAKE_CXX_STANDARD 11)

option(BUILD_TESTS "Build test programs" OFF)

project (mgtools)
include_directories(
    ${mgtools_SOURCE_DIR}/inc
    ${mgtools_SOURCE_DIR}/../../prototype2
    ${mgtools_SOURCE_DIR}/../..)

add_executable(batchreader
    src/batchreader.cpp src/Analyse.cpp src/Args.cpp
    src/MapFile.cpp src/PeakFinder.cpp src/RunSpecParse.cpp src/jsoncpp.cpp
    ../../prototype2/cspec/CSPECChanConv.cpp
    ../../prototype2/cspec/CalibrationFile.cpp
    ../../prototype2/cspec/CSPECData.cpp)

add_executable(cncsread src/cncsread.cpp)

add_executable(genpixids src/genpixids.cpp
    src/DataSave.cpp
    ../../prototype2/cspec/CSPECChanConv.cpp
    ../../prototype2/cspec/CalibrationFile.cpp)


if(BUILD_TESTS)
enable_testing()
find_package(GTest REQUIRED)
include_directories(
    ${GTEST_INCLUDE_DIRS}
    ${mgtools_SOURCE_DIR}/../../prototype2/test
    )

add_executable(PeakFinderTest src/test/PeakFinderTest.cpp src/PeakFinder.cpp)
target_link_libraries(PeakFinderTest ${GTEST_BOTH_LIBRARIES})
add_test(PeakFinderTest PeakFinderTest)

add_executable(HistogramTest src/test/HistogramTest.cpp)
target_link_libraries(HistogramTest ${GTEST_BOTH_LIBRARIES})
add_test(HistogramTest HistogramTest)
endif(BUILD_TESTS)
