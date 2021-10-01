# Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)

include_directories(.)

#=============================================================================
# freia detector module and base test
#=============================================================================
set(freia_common_inc
  FreiaBase.h
  FreiaInstrument.h
  geometry/Config.h
  ${ESS_MODULE_DIR}/multiblade/clustering/EventBuilder.h
  ${ESS_SOURCE_DIR}/common/readout/common/Parser.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Calibration.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/Hybrid.h

  )
set(freia_common_src
  FreiaBase.cpp
  FreiaInstrument.cpp
  geometry/Config.cpp
  ${ESS_MODULE_DIR}/multiblade/clustering/EventBuilder.cpp
  ${ESS_SOURCE_DIR}/common/readout/common/Parser.cpp
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.cpp
  )

set(freia_INC ${freia_common_inc})
set(freia_SRC Freia.cpp ${freia_common_src})
create_module(freia)

#
set(FreiaBaseTest_INC
  ${freia_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
)
set(FreiaBaseTest_SRC
  FreiaBaseTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${freia_common_src}
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
)
create_test_executable(FreiaBaseTest)


##
## FreiaiInstrumentTest Module integration test
##
set(FreiaInstrumentTest_INC
  ${freia_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(FreiaInstrumentTest_SRC
  ${freia_common_src}
  test/FreiaInstrumentTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
)
create_test_executable(FreiaInstrumentTest)
