# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

include_directories(.)

set(BIFROST_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

#=============================================================================
# independent bifrost unit tests
#=============================================================================

set(BifrostGeometryTest_INC
  ${BIFROST_BASE_DIR}/geometry/Geometry.h)

set(BifrostGeometryTest_SRC
  ${BIFROST_BASE_DIR}/geometry/Geometry.cpp
  BifrostGeometryTest.cpp)

create_test_executable(BifrostGeometryTest)



#=============================================================================
# BifrostInstrument
#=============================================================================
set(BifrostInstrumentTest_INC
  ${BIFROST_BASE_DIR}/BifrostInstrument.h
  ${BIFROST_BASE_DIR}/geometry/Config.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h
  ${BIFROST_BASE_DIR}/readout/DataParser.h
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(BifrostInstrumentTest_SRC
  BifrostInstrumentTest.cpp
  ${BIFROST_BASE_DIR}/BifrostInstrument.cpp
  ${BIFROST_BASE_DIR}/geometry/Config.cpp
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.cpp
  ${BIFROST_BASE_DIR}/readout/DataParser.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  )
#create_test_executable(DreamInstrumentTest)
