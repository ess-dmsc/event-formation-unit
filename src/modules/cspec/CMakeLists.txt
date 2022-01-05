# Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file

# add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

#=============================================================================
# cspec detector module and base test
#=============================================================================
set(cspec_common_inc
  CSPECBase.h
  CSPECInstrument.h
  #geometry/Config.h
  #geometry/CSPECChannelMapping.h
  #geometry/AMORChannelMapping.h
  #clustering/EventBuilder.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/CalibFile.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Calibration.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/Hybrid.h

  )
set(cspec_common_src
  CSPECBase.cpp
  CSPECInstrument.cpp
  #geometry/Config.cpp
  #clustering/EventBuilder.cpp
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.cpp
  ${ESS_SOURCE_DIR}/common/readout/vmm3/CalibFile.cpp
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.cpp
  )

set(cspec_INC ${cspec_common_inc})
set(cspec_SRC CSPEC.cpp ${cspec_common_src})
create_module(cspec)

#
set(CSPECBaseTest_INC
  ${cspec_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
)
set(CSPECBaseTest_SRC
  CSPECBaseTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${cspec_common_src}
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
)
create_test_executable(CSPECBaseTest)


##
## CSPECiInstrumentTest Module integration test
##
set(CSPECInstrumentTest_INC
  ${cspec_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(CSPECInstrumentTest_SRC
  ${cspec_common_src}
  test/CSPECInstrumentTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
)
create_test_executable(CSPECInstrumentTest)
