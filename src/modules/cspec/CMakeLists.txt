# Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

#=============================================================================
# cspec detector module and base test
#=============================================================================
set(cspec_common_inc
  CSPECBase.h
  CSPECInstrument.h
  geometry/Config.h
  geometry/CSPECGeometry.h
  geometry/LETGeometry.h
  geometry/Geometry.h
  ${ESS_SOURCE_DIR}/common/reduction/EventBuilder2D.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Calibration.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/Hybrid.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Config.h
  )
set(cspec_common_src
  CSPECBase.cpp
  CSPECInstrument.cpp
  geometry/Config.cpp
  geometry/CSPECGeometry.cpp
  geometry/LETGeometry.cpp
  ${ESS_SOURCE_DIR}/common/reduction/EventBuilder2D.cpp
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.cpp
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.cpp
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Config.cpp
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Calibration.cpp
  )

set(cspec_INC ${cspec_common_inc})
set(cspec_SRC CSPEC.cpp ${cspec_common_src})
create_module(cspec)


set(CSPECBaseTest_INC
  ${cspec_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
)
set(CSPECBaseTest_SRC
  test/CSPECBaseTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${cspec_common_src}
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
)
create_test_executable(CSPECBaseTest)

#
# CSPEC InstrumentTest Module integration test
#
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

