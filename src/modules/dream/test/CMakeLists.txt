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
  ${ESS_MODULE_DIR}/readout/ReadoutParser.h
  ${DREAM_BASE_DIR}/readout/DataParser.h
  ${DREAM_BASE_DIR}/DreamInstrument.h)
set(DreamInstrumentTest_SRC
${ESS_MODULE_DIR}/readout/ReadoutParser.cpp
${DREAM_BASE_DIR}/readout/DataParser.cpp
  ${DREAM_BASE_DIR}/DreamInstrument.cpp
  DreamInstrumentTest.cpp)
create_test_executable(DreamInstrumentTest)


#=============================================================================
# independent dream benchmark tests
#=============================================================================

set(DreamBenchmarkTest_SRC DreamBenchmarkTest.cpp)
set(DreamBenchmarkTest_INC ${DREAM_BASE_DIR}/geometry/DreamGeometry.h)
create_benchmark_executable(DreamBenchmarkTest)
