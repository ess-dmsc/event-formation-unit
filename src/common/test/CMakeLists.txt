
set(BufferTest_SRC
  BufferTest.cpp
  )
create_test_executable(BufferTest)

set(DumpFileTest_SRC
  DumpFileTest.cpp
  )
create_test_executable(DumpFileTest)

set(DetectorTest_SRC
  DetectorTest.cpp
  )
create_test_executable(DetectorTest)

set(EFUArgsTest_SRC
  EFUArgsTest.cpp
  )
create_test_executable(EFUArgsTest)

set(StatisticsTest_SRC
    StatisticsTest.cpp
  )
create_test_executable(StatisticsTest)

set(RuntimeStatTest_INC
    ../RuntimeStat.h
  )
set(RuntimeStatTest_SRC
    RuntimeStatTest.cpp
  )
create_test_executable(RuntimeStatTest)

set(RingBufferTest_SRC
  RingBufferTest.cpp
  )
create_test_executable(RingBufferTest)

set(ESSGeometryTest_SRC
  ESSGeometryTest.cpp
  )
create_test_executable(ESSGeometryTest)

set(BitMathTest_SRC
  BitMathTest.cpp
  )
create_test_executable(BitMathTest)

set(SocketTest_SRC
  SocketTest.cpp
  )
create_test_executable(SocketTest)

set(TestImageUdderTest_SRC
  TestImageUdderTest.cpp
  )
create_test_executable(TestImageUdderTest)

set(FixedSizePoolTest_SRC
  FixedSizePoolTest.cpp
  )
create_test_executable(FixedSizePoolTest)

set(PoolAllocatorTest_SRC
  PoolAllocatorTest.cpp
  )
create_test_executable(PoolAllocatorTest)

# GOOGLE BENCHMARKS
set(ESSGeometryBenchmarkTest_SRC
  ESSGeometryBenchmarkTest.cpp
  )
create_benchmark_executable(ESSGeometryBenchmarkTest)
