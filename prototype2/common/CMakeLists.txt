#=============================================================================
# Common functionality for all detector plugins
#=============================================================================

set(efu_common_SRC
  clustering/AbstractClusterer.h
  clustering/Cluster.h
  clustering/GapClusterer.h
  clustering/Hit.h
  #clustering/Matcher.h
  EV42Serializer.cpp
  NewStats.cpp
  Producer.cpp
  DataSave.cpp
  DetectorModuleRegister.cpp
  TimeString.cpp
  )

set(efu_common_INC
  clustering/Cluster.cpp
  clustering/GapClusterer.cpp
  clustering/Hit.cpp
  #clustering/Matcher.cpp
  ../../libs/include/gccintel.h
  DetectorModuleRegister.h
  Detector.h
  EFUArgs.h
  EV42Serializer.h
  Buffer.h
  NewStats.h
  Producer.h
  Trace.h
  Log.h
  DataSave.h
  DumpFile.h
  TimeString.h
  )

add_subdirectory(clustering)

find_package(CLI11 REQUIRED)
find_package(fmt REQUIRED)

add_library(efu_common STATIC
  ${efu_common_SRC}
  ${efu_common_INC})

target_include_directories(efu_common PUBLIC ${CLI11_INCLUDE_DIR})

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(efu_common "-Wl,--no-as-needed")
endif()

enable_coverage(efu_common)

target_link_libraries(efu_common PUBLIC ${EFU_COMMON_LIBS} fmt::fmt)


#=============================================================================
# Tests
#=============================================================================

if(${CMAKE_COMPILER_IS_GNUCXX})
  set(ProducerTest_SRC
    ProducerTest.cpp
    Producer.cpp)
  set(ProducerTest_INC
    Producer.h)
  create_test_executable(ProducerTest)
else()
  message(STATUS "*** Skipping ProducerTest - requires gcc/linux")
endif()

set(BufferTest_SRC
  BufferTest.cpp)
set(BufferTest_INC
  Buffer.h)
create_test_executable(BufferTest)

set(DataSaveTest_SRC
    DataSaveTest.cpp
    DataSave.cpp
    TimeString.cpp)
set(DataSaveTest_INC
    DataSave.h
    TimeString.h)
create_test_executable(DataSaveTest)

set(DumpFileTest_SRC
  DumpFileTest.cpp
  TimeString.cpp)
set(DumpFileTest_INC
  DumpFile.h
  TimeString.h)
create_test_executable(DumpFileTest)

set(DetectorTest_SRC
  DetectorTest.cpp
  NewStats.cpp)
set(DetectorTest_INC
  DetectorModuleRegister.h
  NewStats.h
  Detector.h)
create_test_executable(DetectorTest)

set(EFUArgsTest_SRC
  EFUArgsTest.cpp
  EFUArgs.cpp)
set(EFUArgsTest_INC
  DetectorModuleRegister.h
  EFUArgs.h)
create_test_executable(EFUArgsTest)

set(EV42SerializerTest_SRC
  EV42SerializerTest.cpp
  EV42Serializer.cpp
  Producer.cpp)
set(EV42SerializerTest_INC
  EV42Serializer.h
  Producer.h)
create_test_executable(EV42SerializerTest)

set(HistsTest_INC
  Hists.h
)
set(HistsTest_SRC
  HistsTest.cpp
  Hists.cpp
)
create_test_executable(HistsTest)

set(NewStatsTest_SRC
  NewStatsTest.cpp
  NewStats.cpp)
set(NewStatsTest_INC
  NewStats.h)
create_test_executable(NewStatsTest)

set(ReadoutSerializerTest_SRC
  ReadoutSerializerTest.cpp
  ReadoutSerializer.cpp
  Producer.cpp)
set(ReadoutSerializerTest_INC
  ReadoutSerializer.h
  Producer.cpp)
create_test_executable(ReadoutSerializerTest)

set(RingBufferTest_SRC
  RingBufferTest.cpp)
set(RingBufferTest_INC
  RingBuffer.h)
create_test_executable(RingBufferTest)

set(VersionTest_SRC
  VersionTest.cpp)
set(VersionTest_INC
  Version.h)
create_test_executable(VersionTest)

set(ESSGeometryTest_SRC ESSGeometryTest.cpp)
#set(ESSGeometryTest_INC ESSGeometry.h)
create_test_executable(ESSGeometryTest)

# GOOGLE BENCHMARKS
set(ESSGeometryBenchmarkTest_SRC ESSGeometryBenchmarkTest.cpp)
create_benchmark_executable(ESSGeometryBenchmarkTest)
