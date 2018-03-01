cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project("AdcReadout" LANGUAGES CXX C)

include_directories(${CMAKE_SOURCE_DIR})

set(AdcReadoutCore_SRC
  AdcReadoutCore.cpp
  AdcParse.cpp
  PeakFinder.cpp
  AdcDataProcessor.cpp
  SampleProcessing.cpp
  AdcSettings.cpp
)

set(AdcReadout_SRC
  AdcReadout.cpp
  ${AdcReadoutCore_SRC}
)

set(AdcReadoutCore_INC
  AdcReadoutCore.h
  AdcParse.h
  AdcBufferElements.h
  atomicops.h
  CircularBuffer.h
  readerwriterqueue.h
  PeakFinder.h
  AdcDataProcessor.h
  SampleProcessing.h
  AdcSettings.h
)

set(AdcReadout_INC
  ${AdcReadoutCore_INC}
)

create_module(AdcReadout "")

enable_testing()

include_directories(..)

set(AdcReadoutTest_SRC
  test/UnitTests.cpp
  test/CircularBufferTest.cpp
  test/AdcReadoutTest.cpp
  test/AdcParseTest.cpp
  test/TestUDPServer.cpp
  test/PeakFinderTest.cpp
  test/AdcDataProcessorTest.cpp
  test/SampleProcessingTest.cpp
  test/CLIArgumentsTest.cpp
  ${AdcReadoutCore_SRC})

set(AdcReadoutTest_INC
  test/TestUDPServer.h
  ${AdcReadoutCore_INC}
)

find_package(ASIO)

if(${ASIO_FOUND})
  
  create_test_executable(AdcReadoutTest ${LibRDKafka_LIBRARIES})
  get_filename_component(TEST_PACKET_PATH "test/test_packet_1.dat" DIRECTORY)
  target_compile_definitions(AdcReadoutTest PRIVATE TEST_PACKET_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_PACKET_PATH}/")
  message(STATUS "Test ADC packets in path: ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_PACKET_PATH}/")

  add_dependencies(AdcReadoutTest efu_common eventlib)
  target_link_libraries(AdcReadoutTest efu_common eventlib)
  target_include_directories(AdcReadoutTest PRIVATE ${ASIO_INCLUDE_DIR})
  add_subdirectory("chopper_integration_test")
endif()
