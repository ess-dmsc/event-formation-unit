#=============================================================================
# Common functionality for all detector plugins
#=============================================================================
set(efu_common_SRC
  FBSerializer.cpp
  NewStats.cpp
  Producer.cpp
  DataSave.cpp
  DetectorModuleRegister.cpp
  )
set(efu_common_INC
  ../../libs/include/gccintel.h
  DetectorModuleRegister.h
  Detector.h
  EFUArgs.h
  FBSerializer.h
  NewStats.h
  Producer.h
  Trace.h
  DataSave.h
  )

find_package(CLI11 REQUIRED)

add_library(efu_common STATIC
  ${efu_common_SRC}
  ${efu_common_INC})

target_include_directories(efu_common PUBLIC ${CLI11_INCLUDE_DIR})

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(efu_common "-Wl,--no-as-needed")
endif()

enable_coverage(efu_common)

target_link_libraries(efu_common
    ${EFU_COMMON_LIBS})

set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} efu_common PARENT_SCOPE)


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

set(DataSaveTest_SRC
    DataSaveTest.cpp
    DataSave.cpp)
set(DataSaveTest_INC
    DataSave.h)
create_test_executable(DataSaveTest)

set(DetectorTest_SRC
  DetectorModuleRegister.cpp
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
  EFUArgs.h)
create_test_executable(EFUArgsTest)

set(FBSerializerTest_SRC
  FBSerializerTest.cpp
  FBSerializer.cpp
  Producer.cpp)
set(FBSerializerTest_INC
  FBSerializer.h
  Producer.h)
create_test_executable(FBSerializerTest)

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
