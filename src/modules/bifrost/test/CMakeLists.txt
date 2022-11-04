# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

include_directories(.)

set(BIFROST_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

#=============================================================================
# Unit test - geometry/Geometry.cpp
#=============================================================================
set(BifrostGeometryTest_INC
  ${BIFROST_BASE_DIR}/geometry/Geometry.h)

set(BifrostGeometryTest_SRC
  ${BIFROST_BASE_DIR}/geometry/Geometry.cpp
  BifrostGeometryTest.cpp)

create_test_executable(BifrostGeometryTest)


#=============================================================================
# Unit test - readout/DataParser.cpp
#=============================================================================
set(BifrostDataParserTest_INC
  ${BIFROST_BASE_DIR}/readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h)
set(BifrostDataParserTest_SRC
  ${BIFROST_BASE_DIR}/readout/DataParser.cpp
  DataParserTest.cpp)

create_test_executable(BifrostDataParserTest)


#=============================================================================
# BifrostInstrument
#=============================================================================
set(BifrostInstrumentTest_INC
  ${BIFROST_BASE_DIR}/BifrostInstrument.h
  ${BIFROST_BASE_DIR}/geometry/Config.h
  ${BIFROST_BASE_DIR}/geometry/Geometry.h
  ${BIFROST_BASE_DIR}/readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(BifrostInstrumentTest_SRC
  BifrostInstrumentTest.cpp
  ${BIFROST_BASE_DIR}/BifrostInstrument.cpp
  ${BIFROST_BASE_DIR}/geometry/Geometry.cpp
  ${BIFROST_BASE_DIR}/readout/DataParser.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  )
create_test_executable(BifrostInstrumentTest)
