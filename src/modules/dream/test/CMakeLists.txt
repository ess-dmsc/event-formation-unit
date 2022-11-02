include_directories(.)

set(DREAM_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")


#=============================================================================
# independent dream unit tests
#=============================================================================

set(SUMOGeometryTest_INC
  ${DREAM_BASE_DIR}/geometry/SUMO.h)
set(SUMOGeometryTest_SRC
  SUMOGeometryTest.cpp)
create_test_executable(SUMOGeometryTest)


set(CDTGeometryTest_INC
  ${DREAM_BASE_DIR}/geometry/CDTGeometry.h)
set(CDTGeometryTest_SRC
  ${DREAM_BASE_DIR}/geometry/CDTGeometry.cpp
  CDTGeometryTest.cpp)
create_test_executable(CDTGeometryTest)


set(DreamConfigTest_INC
  ${DREAM_BASE_DIR}/geometry/Config.h
  )
set(DreamConfigTest_SRC
  ${DREAM_BASE_DIR}/geometry/Config.cpp
  ConfigTest.cpp
  )
create_test_executable(DreamConfigTest)

set(CuboidTest_INC
  ${DREAM_BASE_DIR}/geometry/Cuboid.h
  )
set(CuboidTest_SRC
  CuboidTest.cpp
  )
create_test_executable(CuboidTest)


set(DreamInstrumentTest_INC
  ${DREAM_BASE_DIR}/DreamInstrument.h
  ${DREAM_BASE_DIR}/geometry/Config.h
  ${DREAM_BASE_DIR}/geometry/CDTGeometry.h
  ${DREAM_BASE_DIR}/readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(DreamInstrumentTest_SRC
  DreamInstrumentTest.cpp
  ${DREAM_BASE_DIR}/DreamInstrument.cpp
  ${DREAM_BASE_DIR}/geometry/Config.cpp
  ${DREAM_BASE_DIR}/geometry/CDTGeometry.cpp
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
