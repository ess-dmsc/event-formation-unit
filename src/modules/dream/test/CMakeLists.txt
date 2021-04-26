include_directories(.)

set(DREAM_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")


#=============================================================================
# independent dream unit tests
#=============================================================================

set(DreamGeometryTest_INC
  ${DREAM_BASE_DIR}/geometry/DreamGeometry.h)
set(DreamGeometryTest_SRC DreamGeometryTest.cpp)
create_test_executable(DreamGeometryTest)


set(DreamInstrumentTest_INC
  ${DREAM_BASE_DIR}/geometry/Config.cpp
  ${ESS_MODULE_DIR}/readout/ReadoutParser.h
  ${DREAM_BASE_DIR}/readout/DataParser.h
  ${DREAM_BASE_DIR}/DreamInstrument.h)
set(DreamInstrumentTest_SRC
  ${DREAM_BASE_DIR}/geometry/Config.cpp
  ${ESS_MODULE_DIR}/readout/ReadoutParser.cpp
  ${DREAM_BASE_DIR}/readout/DataParser.cpp
  ${DREAM_BASE_DIR}/DreamInstrument.cpp
  DreamInstrumentTest.cpp)
create_test_executable(DreamInstrumentTest)



set(DreamDataParserTest_INC
  ../readout/DataParser.h
  ../../readout/ReadoutParser.h)
set(DreamDataParserTest_SRC
  ../readout/DataParser.cpp
  DataParserTest.cpp)
create_test_executable(DreamDataParserTest)


#=============================================================================
# independent dream benchmark tests
#=============================================================================

set(DreamBenchmarkTest_SRC DreamBenchmarkTest.cpp)
set(DreamBenchmarkTest_INC ${DREAM_BASE_DIR}/geometry/DreamGeometry.h)
create_benchmark_executable(DreamBenchmarkTest)
