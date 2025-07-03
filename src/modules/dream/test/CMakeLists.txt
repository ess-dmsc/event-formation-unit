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

set(PADetectorTest_INC
  ${DREAM_BASE_DIR}/geometry/PADetector.h)
set(PADetectorTest_SRC
  PADetectorTest.cpp)
create_test_executable(PADetectorTest)


set(DreamGeometryTest_INC
  ${DREAM_BASE_DIR}/geometry/DreamGeometry.h
  ${DREAM_BASE_DIR}/geometry/HeimdalGeometry.h
  ${DREAM_BASE_DIR}/geometry/MagicGeometry.h)
set(DreamGeometryTest_SRC
  ${DREAM_BASE_DIR}/geometry/DreamGeometry.cpp
  ${DREAM_BASE_DIR}/geometry/HeimdalGeometry.cpp
  ${DREAM_BASE_DIR}/geometry/MagicGeometry.cpp
  DreamGeometryTest.cpp)
create_test_executable(DreamGeometryTest)

set(MagicGeometryTest_INC
  ${DREAM_BASE_DIR}/geometry/MagicGeometry.h)
set(MagicGeometryTest_SRC
  ${DREAM_BASE_DIR}/geometry/MagicGeometry.cpp
  MagicGeometryTest.cpp)
create_test_executable(MagicGeometryTest)

set(HeimdalGeometryTest_INC
  ${DREAM_BASE_DIR}/geometry/HeimdalGeometry.h)
set(HeimdalGeometryTest_SRC
  ${DREAM_BASE_DIR}/geometry/HeimdalGeometry.cpp
  HeimdalGeometryTest.cpp)
create_test_executable(HeimdalGeometryTest)


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
  ${DREAM_BASE_DIR}/geometry/DreamGeometry.h
  ${DREAM_BASE_DIR}/geometry/HeimdalGeometry.h
  ${DREAM_BASE_DIR}/geometry/MagicGeometry.h
  ${DREAM_BASE_DIR}/readout/DataParser.h
  )
set(DreamInstrumentTest_SRC
  DreamInstrumentTest.cpp
  ${DREAM_BASE_DIR}/DreamInstrument.cpp
  ${DREAM_BASE_DIR}/geometry/Config.cpp
  ${DREAM_BASE_DIR}/geometry/DreamGeometry.cpp
  ${DREAM_BASE_DIR}/geometry/HeimdalGeometry.cpp
  ${DREAM_BASE_DIR}/geometry/MagicGeometry.cpp
  ${DREAM_BASE_DIR}/readout/DataParser.cpp
  )

set(DreamInstrumentTest_LIB efu_essreadout)
create_test_executable(DreamInstrumentTest)



set(DreamDataParserTest_INC
  ../readout/DataParser.h
  ${ESS_COMMON_DIR}/readout/ess/Parser.h)
set(DreamDataParserTest_SRC
  ../readout/DataParser.cpp
  DataParserTest.cpp)
create_test_executable(DreamDataParserTest)
