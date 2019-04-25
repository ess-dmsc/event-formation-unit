find_package(Trompeloeil REQUIRED)
set(ProducerTest_SRC
ProducerTest.cpp)
set(ProducerTest_INC
KafkaMocks.h)
create_test_executable(ProducerTest)
target_include_directories(ProducerTest PRIVATE ${Trompeloeil_INCLUDE_DIR})


set(BufferTest_SRC
  BufferTest.cpp)
create_test_executable(BufferTest)

set(DataSaveTest_SRC
    DataSaveTest.cpp
  )
create_test_executable(DataSaveTest)

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

set(EV42SerializerTest_SRC
  EV42SerializerTest.cpp
)
create_test_executable(EV42SerializerTest)

set(HistsTest_SRC
  HistsTest.cpp
)
create_test_executable(HistsTest)

set(NewStatsTest_SRC
NewStatsTest.cpp)
create_test_executable(NewStatsTest)

set(ReadoutSerializerTest_SRC
ReadoutSerializerTest.cpp)
create_test_executable(ReadoutSerializerTest)

set(RingBufferTest_SRC
  RingBufferTest.cpp)
create_test_executable(RingBufferTest)

set(VersionTest_SRC
  VersionTest.cpp)
create_test_executable(VersionTest)

set(ESSGeometryTest_SRC ESSGeometryTest.cpp)
create_test_executable(ESSGeometryTest)

set(HistSerializerTest_SRC
  HistSerializerTest.cpp
  )
create_test_executable(HistSerializerTest)

# GOOGLE BENCHMARKS
set(ESSGeometryBenchmarkTest_SRC ESSGeometryBenchmarkTest.cpp)
create_benchmark_executable(ESSGeometryBenchmarkTest)
