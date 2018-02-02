#
# EFU Tests
#
if(${CMAKE_COMPILER_IS_GNUCXX})
  set(ParserTest_LIB_SRC
    efu/ParserTest.cpp
    test/SyscallWrappers.cpp
    common/NewStats.cpp)
  set(ParserTest_LIB_INC
    common/NewStats.h)
  add_library(ParserTest_LIB STATIC
    ${ParserTest_LIB_SRC})
  add_compile_flags(ParserTest_LIB
    "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")

  set(ParserTest_SRC
    efu/Parser.cpp
    common/EFUArgs.cpp
    $<TARGET_OBJECTS:calibfile>)
  set(ParserTest_INC
    efu/Parser.h
    common/EFUArgs.h)
  create_test_executable(ParserTest "ParserTest_LIB")
  add_compile_flags(ParserTest
    "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
  add_linker_flags(ParserTest
    "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
else()
  message(STATUS "*** Skipping ParserTest - requires gcc/linux")
endif()


#
# common/ Tests
#

if(${DUMPTOFILE})
  set(DataSaveTest_SRC
    common/DataSaveTest.cpp
    common/DataSave.cpp)
  set(DataSaveTest_INC
    common/DataSave.h)
  create_test_executable(DataSaveTest "")
endif()

set(DetectorTest_SRC
  common/DetectorTest.cpp
  common/NewStats.cpp)
set(DetectorTest_INC
  common/NewStats.h
  common/Detector.h)
create_test_executable(DetectorTest "")

set(EFUArgsTest_SRC
  common/EFUArgsTest.cpp
  common/EFUArgs.cpp)
set(EFUArgsTest_INC
  common/EFUArgs.h)
create_test_executable(EFUArgsTest "")

set(FBSerializerTest_SRC
  common/FBSerializerTest.cpp
  common/FBSerializer.cpp
  common/Producer.cpp)
set(FBSerializerTest_INC
  common/FBSerializer.h
  common/Producer.h)
create_test_executable(FBSerializerTest "")

set(NewStatsTest_SRC
  common/NewStatsTest.cpp
  common/NewStats.cpp)
set(NewStatsTest_INC
  common/NewStats.h)
create_test_executable(NewStatsTest "")

if(${CMAKE_COMPILER_IS_GNUCXX})
  set(ProducerTest_SRC
    common/ProducerTest.cpp
    common/Producer.cpp)
  set(ProducerTest_INC
    common/Producer.h)
  create_test_executable(ProducerTest "")
else()
  message(STATUS "*** Skipping ProducerTest - requires gcc/linux")
endif()

set(ReadoutSerializerTest_SRC
  common/ReadoutSerializerTest.cpp
  common/ReadoutSerializer.cpp
  common/Producer.cpp)
set(ReadoutSerializerTest_INC
  common/ReadoutSerializer.h
  common/Producer.cpp)
create_test_executable(ReadoutSerializerTest "")

set(RingBufferTest_SRC
  common/RingBufferTest.cpp)
set(RingBufferTest_INC
  common/RingBuffer.h)
create_test_executable(RingBufferTest "")

set(VersionTest_SRC
  common/VersionTest.cpp)
set(VersionTest_INC
  common/Version.h)
create_test_executable(VersionTest "")



#
# ESS Readout System Tests
#
set(ReadoutTest_SRC
  readout/ReadoutTest.cpp
  readout/Readout.cpp)
set(ReadoutTest_INC
  test/TestBase.h
  readout/Readout.h
  readout/ReadoutTestData.h)
create_test_executable(ReadoutTest "")

set(ReadoutDummyTest_SRC
  readout/ReadoutDummyTest.cpp
  readout/Readout.cpp
  readout/ReadoutDummy.cpp)
set(ReadoutDummyTest_INC
  test/TestBase.h
  readout/Readout.h
  readout/ReadoutDummy.h
  readout/ReadoutTestData.h)
create_test_executable(ReadoutDummyTest "")


#
# Libs unit tests
#

include_directories(..)

set(CounterTest_SRC
  ../libs/test/Counter/testmain.cpp
  ../libs/test/Counter/CounterTest.cpp)
set(CounterTest_INC
  ../libs/include/Counter.h)
create_test_executable(CounterTest "")

set(StatCounterTest_SRC
  ../libs/test/Counter/testmain.cpp
  ../libs/test/Counter/StatCounterTest.cpp)
set(StatCounterTest_INC
  ../libs/include/StatCounter.h)
create_test_executable(StatCounterTest "")


