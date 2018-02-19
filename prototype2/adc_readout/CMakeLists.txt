cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project("AdcReadout" LANGUAGES CXX C)

include_directories(${CMAKE_SOURCE_DIR})

set(AdcReadout_SRC
  AdcReadout.cpp
  AdcParse.cpp
  PeakFinder.cpp
  AdcDataProcessor.cpp
)

set(AdcReadout_INC
  AdcReadout.h
  AdcParse.h
  AdcBufferElements.h
  atomicops.h
  CircularBuffer.h
  readerwriterqueue.h
  PeakFinder.h
  AdcDataProcessor.h
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
  ${AdcReadout_SRC})

set(AdcReadoutTest_INC
  test/TestUDPServer.h
  ${AdcReadout_INC}
)

find_package(ASIO)

if(${ASIO_FOUND})
  
  create_test_executable(AdcReadoutTest ${LibRDKafka_LIBRARIES})
  get_filename_component(TEST_PACKET_FILE "test/test_packet_1.dat" ABSOLUTE)
  add_custom_command(
          TARGET AdcReadoutTest POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
                  ${TEST_PACKET_FILE}
                  "$<TARGET_FILE_DIR:AdcReadoutTest>/")

  add_dependencies(AdcReadoutTest efu_common eventlib)
  target_link_libraries(AdcReadoutTest efu_common eventlib)
  target_include_directories(AdcReadoutTest PRIVATE ${ASIO_INCLUDE_DIR})
  add_subdirectory("chopper_integration_test")
endif()
