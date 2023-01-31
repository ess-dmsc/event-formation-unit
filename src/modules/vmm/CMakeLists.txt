# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
include_directories(.)

#=============================================================================
# vmm detector module and base test
#=============================================================================
set(vmm_common_inc
  VMMBase.h
  VMMInstrument.h
  geometry/Config.h
  geometry/Geometry.h
  ${ESS_MODULE_DIR}/freia/geometry/FreiaGeometry.h
  ${ESS_MODULE_DIR}/freia/geometry/AMORGeometry.h
  # ${ESS_MODULE_DIR}/nmx/geometry/NMXGeometry.h
  ${ESS_COMMON_DIR}/readout/vmm3/VMM3Parser.h
  )

set(vmm_common_src
  VMMBase.cpp
  VMMInstrument.cpp
  geometry/Config.cpp
  ${ESS_MODULE_DIR}/freia/geometry/FreiaGeometry.cpp
  # ${ESS_MODULE_DIR}/freia/geometry/AMORGeometry.cpp
  ${ESS_COMMON_DIR}/readout/vmm3/VMM3Parser.cpp
  )

  add_library(vmm_common STATIC
  ${vmm_common_src}
  ${vmm_common_inc}
  )

set(vmm_INC ${vmm_common_inc})
set(vmm_SRC
  ${vmm_common_src}
  main.cpp
  )
set(vmm_LIB efu_reduction efu_essreadout)
create_executable(vmm)

#
set(VMMBaseTest_INC
  ${vmm_common_inc}
)
set(VMMBaseTest_SRC
  VMMBaseTest.cpp
  ${vmm_common_src}
)
create_test_executable(VMMBaseTest)


##
## VMMiInstrumentTest Module integration test
##
set(VMMInstrumentTest_INC
  ${vmm_common_inc}
  )
set(VMMInstrumentTest_SRC
  ${vmm_common_src}
  test/VMMInstrumentTest.cpp
)
create_test_executable(VMMInstrumentTest)
