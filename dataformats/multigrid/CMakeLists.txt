cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
set(CMAKE_INSTALL_PREFIX "")

option(BUILD_TESTS "Build test programs" ON)

project (mgtools)

set(CMAKE_CXX_FLAGS " -O3 -std=c++11 -Werror -Wall -Wpedantic -Wextra -Wmissing-include-dirs")

include_directories(
    ${mgtools_SOURCE_DIR}/inc
    ${mgtools_SOURCE_DIR}/../../prototype2
    ${mgtools_SOURCE_DIR}/../..)


add_executable(batchreader
    src/batchreader.cpp src/Analyse.cpp src/Args.cpp
    src/MapFile.cpp src/PeakFinder.cpp src/RunSpecParse.cpp src/jsoncpp.cpp
    ../../prototype2/multigrid/mgcncs/ChanConv.cpp
    ../../prototype2/multigrid/mgcncs/CalibrationFile.cpp
    ../../prototype2/multigrid/mgcncs/Data.cpp)

add_executable(cncsread src/cncsread.cpp)

add_executable(genpixids src/genpixids.cpp
    src/DataSave.cpp
    ../../prototype2/multigrid/mgcncs/ChanConv.cpp
    ../../prototype2/multigrid/mgcncs/CalibrationFile.cpp)

#
# Install Files
#
install(TARGETS cncsread genpixids batchreader DESTINATION tools)


#
# Unit tests
#
set(unit_test_targets "")
if(BUILD_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  include_directories(
    ${GTEST_INCLUDE_DIRS}
    ${mgtools_SOURCE_DIR}/../../prototype2/test
    )

  add_executable(PeakFinderTest src/test/PeakFinderTest.cpp src/PeakFinder.cpp)
  target_link_libraries(PeakFinderTest ${GTEST_BOTH_LIBRARIES} pthread)
  set(unit_test_targets PeakFinderTest ${unit_test_targets} )
  add_test(NAME regular_PeakFinderTest COMMAND PeakFinderTest "--gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/PeakFinderTesttest.xml")

  add_executable(HistogramTest src/test/HistogramTest.cpp)
  target_link_libraries(HistogramTest ${GTEST_BOTH_LIBRARIES} pthread)
  set(unit_test_targets HistogramTest ${unit_test_targets} )
  add_test(NAME regular_HistogramTest COMMAND HistogramTest "--gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/HistogramTesttest.xml")
endif(BUILD_TESTS)

add_custom_target(runtest COMMAND ${CMAKE_CTEST_COMMAND} -V -R regular_* DEPENDS ${unit_test_targets})
