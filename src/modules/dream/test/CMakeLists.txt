include_directories(.)

set(DREAM_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")


#=============================================================================
# independent dream unit tests
#=============================================================================

set(DreamGeometryTest_INC
  ${DREAM_BASE_DIR}/geometry/Geometry.h)
set(DreamGeometryTest_SRC
  DreamGeometryTest.cpp)
create_test_executable(DreamGeometryTest)


set(CDTGeometryTest_INC
  ${DREAM_BASE_DIR}/geometry/CDTGeometry.h)
set(CDTGeometryTest_SRC
  CDTGeometryTest.cpp)
create_test_executable(CDTGeometryTest)


set(DreamConfigTest_INC
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${DREAM_BASE_DIR}/geometry/Config.h
  )
set(DreamConfigTest_SRC
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${DREAM_BASE_DIR}/geometry/Config.cpp
  ConfigTest.cpp
  )
create_test_executable(DreamConfigTest)


set(DreamInstrumentTest_INC
  ${DREAM_BASE_DIR}/DreamInstrument.h
  ${DREAM_BASE_DIR}/geometry/Config.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h
  ${DREAM_BASE_DIR}/readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(DreamInstrumentTest_SRC
  DreamInstrumentTest.cpp
  ${DREAM_BASE_DIR}/DreamInstrument.cpp
  ${DREAM_BASE_DIR}/geometry/Config.cpp
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.cpp
  ${DREAM_BASE_DIR}/readout/DataParser.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  )
create_test_executable(DreamInstrumentTest)



set(DreamDataParserTest_INC
  ../readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h)
set(DreamDataParserTest_SRC
  ../readout/DataParser.cpp
  DataParserTest.cpp)
create_test_executable(DreamDataParserTest)
