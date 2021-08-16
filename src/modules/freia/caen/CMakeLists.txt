

# Create excecutable for the test of geometry
set(MBGeometryTest_INC
  MBGeometry.h)
set(MBGeometryTest_SRC
  MBGeometryTest.cpp)
create_test_executable(MBGeometryTest)


#
# Tests relating to event building and benchmarks
#

# EventBuilderBenchmarkTest.cpp
set(MBEventBuilderBenchmarkTest_INC
  ../clustering/EventBuilder.h
)
set(MBEventBuilderBenchmarkTest_SRC
  ../clustering/EventBuilder.cpp
EventBuilderBenchmarkTest.cpp)
create_benchmark_executable(MBEventBuilderBenchmarkTest)
