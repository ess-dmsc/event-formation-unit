set(MBReadoutTest_SRC
  Readout.cpp
  ReadoutTest.cpp
  )
set(MBReadoutTest_INC
  Readout.h
  )
create_test_executable(MBReadoutTest SKIP_MEMGRIND)

# GOOGLE BENCHMARK
set(MBDataParserBenchmarkTest_INC)
set(MBDataParserBenchmarkTest_SRC
  DataParser.cpp
  DataParserBenchmarkTest.cpp)
create_benchmark_executable(MBDataParserBenchmarkTest)
if(GOOGLE_BENCHMARK)
  target_link_libraries(MBDataParserBenchmarkTest PRIVATE h5cpp)
endif()

# Create excecutable for the test of geometry
set(MBDataParserTest_INC
  DataParser.h)
set(MBDataParserTest_SRC
  DataParser.cpp
  DataParserTest.cpp)
create_test_executable(MBDataParserTest)


# Create excecutable for the test of geometry
set(MB16DetectorTest_INC MB16Detector.h)
set(MB16DetectorTest_SRC MB16DetectorTest.cpp)
create_test_executable(MB16DetectorTest)

#
set(MBConfigTest_INC
  ../../../prototype2/common/DataSave.h
  Config.h)
set(MBConfigTest_SRC
  ../../../prototype2/common/DataSave.cpp
  Config.cpp
  ConfigTest.cpp)
create_test_executable(MBConfigTest)
