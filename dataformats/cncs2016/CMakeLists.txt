cmake_minimum_required (VERSION 3.5)
set (CMAKE_CXX_STANDARD 11)

option(BUILD_TESTS "Build test programs" ON)


project (mgtools)

set (CMAKE_INSTALL_PREFIX "/tmp/tools")
set(CMAKE_CXX_FLAGS_DEBUG " -O3 -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs")
set(CMAKE_CXX_FLAGS_RELEASE " -O3 -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs")

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

#
# Install Files
#
INSTALL(TARGETS cncsread genpixids batchreader DESTINATION tools)


#
# Unit tests
#
if(BUILD_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  include_directories(
    ${GTEST_INCLUDE_DIRS}
    ${mgtools_SOURCE_DIR}/../../prototype2/test
    )

  add_executable(PeakFinderTest src/test/PeakFinderTest.cpp src/PeakFinder.cpp)
  target_link_libraries(PeakFinderTest ${GTEST_BOTH_LIBRARIES} pthread)
  add_test(PeakFinderTest PeakFinderTest)

  add_executable(HistogramTest src/test/HistogramTest.cpp)
  target_link_libraries(HistogramTest ${GTEST_BOTH_LIBRARIES} pthread)
  add_test(HistogramTest HistogramTest)
endif(BUILD_TESTS)
