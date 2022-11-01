# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

#=============================================================================
# nmx detector module and base test
#=============================================================================
set(nmx_common_inc
  NMXBase.h
  NMXInstrument.h
  geometry/Config.h
  geometry/NMXGeometry.h
  geometry/Geometry.h
  ${ESS_SOURCE_DIR}/common/reduction/EventBuilder2D.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Calibration.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/Hybrid.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Config.h
  )
set(nmx_common_src
  NMXBase.cpp
  NMXInstrument.cpp
  geometry/Config.cpp
  geometry/NMXGeometry.cpp
  ${ESS_SOURCE_DIR}/common/reduction/EventBuilder2D.cpp
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.cpp
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.cpp
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Config.cpp
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Calibration.cpp
  )

set(nmx_INC ${EFUCOMMON_INC} ${nmx_common_inc})
set(nmx_SRC ${EFUCOMMON_SRC} ${nmx_common_src})
create_executable(nmx)
target_compile_definitions(nmx PUBLIC EFU_NMX)


set(NMXBaseTest_INC
  ${nmx_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
)
set(NMXBaseTest_SRC
  test/NMXBaseTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${nmx_common_src}
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
)
create_test_executable(NMXBaseTest)


# NMX InstrumentTest Module integration test

set(NMXInstrumentTest_INC
  ${nmx_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(NMXInstrumentTest_SRC
  ${nmx_common_src}
  test/NMXInstrumentTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
)
create_test_executable(NMXInstrumentTest)
