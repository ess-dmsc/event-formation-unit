cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project("AdcReadout" LANGUAGES CXX C)

include_directories(${CMAKE_SOURCE_DIR})

set(AdcReadout_SRC
  AdcReadout.cpp
  AdcParse.cpp)

set(AdcReadout_INC
  AdcReadout.h
  AdcParse.h
  AdcBufferElements.h
  atomicops.h
  CircularBuffer.h
  readerwriterqueue.h)

create_module(AdcReadout "")

enable_testing()

include_directories(..)

set(AdcReadoutTest_SRC
  test/UnitTests.cpp
  test/CircularBufferTest.cpp
  test/AdcReadoutTest.cpp
  test/AdcParseTest.cpp
  test/TestUDPServer.cpp
  ${AdcReadout_SRC})

set(AdcReadoutTest_INC
  test/TestUDPServer.h
  ${AdcReadout_INC})
  
create_test_executable(AdcReadoutTest "")
