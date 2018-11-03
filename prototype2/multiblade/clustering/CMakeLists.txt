#=============================================================================
# Tests
#=============================================================================

# GOOGLE BENCHMARK
set(MBEventBuilderBenchmarkTest_INC
  EventBuilder.h
  TestData.h)
set(MBEventBuilderBenchmarkTest_SRC
  EventBuilder.cpp
  EventBuilderBenchmarkTest.cpp
  )
create_benchmark_executable(MBEventBuilderBenchmarkTest)
if(GOOGLE_BENCHMARK)
  find_package(fmt REQUIRED)
  target_link_libraries(MBEventBuilderBenchmarkTest PRIVATE fmt::fmt)
endif()

# Create excecutable for the test of counters
set(MBEventBuilderTest_INC
  EventBuilder.h
  TestData.h)
set(MBEventBuilderTest_SRC
  EventBuilder.cpp
  EventBuilderCounterTest.cpp
  )
create_test_executable(MBEventBuilderTest)

# Create excecutable for the test of clustering
# set(MBClusterTest_INC ${test_INC})
# set(MBClusterTest_SRC
#  EventBuilder.cpp
#  EventBuilderClusteringTest.cpp
#   )
# create_test_executable(MBClusterTest)

#
# MJC Unit tests below
#
