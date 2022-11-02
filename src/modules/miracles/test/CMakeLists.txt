# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

include_directories(.)

set(MIRACLES_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

#=============================================================================
# Unit test - geometry/Geometry.cpp
#=============================================================================
set(MiraclesGeometryTest_INC
  ${MIRACLES_BASE_DIR}/geometry/Geometry.h)

set(MiraclesGeometryTest_SRC
  ${MIRACLES_BASE_DIR}/geometry/Geometry.cpp
  MiraclesGeometryTest.cpp)

create_test_executable(MiraclesGeometryTest)


#=============================================================================
# Unit test - readout/DataParser.cpp
#=============================================================================
set(MiraclesDataParserTest_INC
  ${MIRACLES_BASE_DIR}/readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h)
set(MiraclesDataParserTest_SRC
  ${MIRACLES_BASE_DIR}/readout/DataParser.cpp
  DataParserTest.cpp)

create_test_executable(MiraclesDataParserTest)


#=============================================================================
# MiraclesInstrument
#=============================================================================
set(MiraclesInstrumentTest_INC
  ${MIRACLES_BASE_DIR}/MiraclesInstrument.h
  ${MIRACLES_BASE_DIR}/geometry/Config.h
  ${MIRACLES_BASE_DIR}/geometry/Geometry.h
  ${MIRACLES_BASE_DIR}/readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(MiraclesInstrumentTest_SRC
  MiraclesInstrumentTest.cpp
  ${MIRACLES_BASE_DIR}/MiraclesInstrument.cpp
  ${MIRACLES_BASE_DIR}/geometry/Geometry.cpp
  ${MIRACLES_BASE_DIR}/readout/DataParser.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  )
create_test_executable(MiraclesInstrumentTest)
