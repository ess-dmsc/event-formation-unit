include_directories(.)

set(DREAM_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")


#=============================================================================
# independent dream unit tests
#=============================================================================

set(DreamGeometryTest_INC
  ${DREAM_BASE_DIR}/geometry/DreamGeometry.h
  )
set(DreamGeometryTest_SRC
  DreamGeometryTest.cpp
  )
create_test_executable(DreamGeometryTest)


set(DreamConfigTest_INC
  ${ESS_SOURCE_DIR}/test/SaveBuffer.h
  ${DREAM_BASE_DIR}/geometry/Config.h
  )
set(DreamConfigTest_SRC
  ${ESS_SOURCE_DIR}/test/SaveBuffer.cpp
  ${DREAM_BASE_DIR}/geometry/Config.cpp
  ConfigTest.cpp
  )
create_test_executable(DreamConfigTest)


set(DreamInstrumentTest_INC
  ${DREAM_BASE_DIR}/DreamInstrument.h
  ${DREAM_BASE_DIR}/geometry/Config.h
  ${ESS_MODULE_DIR}/readout/common/Parser.h
  ${DREAM_BASE_DIR}/readout/DataParser.h
  ${ESS_SOURCE_DIR}/test/SaveBuffer.h
  )
set(DreamInstrumentTest_SRC
  DreamInstrumentTest.cpp
  ${DREAM_BASE_DIR}/DreamInstrument.cpp
  ${DREAM_BASE_DIR}/geometry/Config.cpp
  ${ESS_MODULE_DIR}/readout/common/Parser.cpp
  ${DREAM_BASE_DIR}/readout/DataParser.cpp
  ${ESS_SOURCE_DIR}/test/SaveBuffer.cpp
  )
create_test_executable(DreamInstrumentTest)



set(DreamDataParserTest_INC
  ../readout/DataParser.h
  ../../readout/common/Parser.h)
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
