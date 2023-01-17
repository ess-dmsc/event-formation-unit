# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

#=============================================================================
# vmm detector module and base test
#=============================================================================
set(vmm_common_inc
  VMMBase.h
  VMMInstrument.h
  geometry/Config.h
  geometry/VMMChannelMapping.h
  geometry/AMORChannelMapping.h
  )
set(vmm_common_src
  VMMBase.cpp
  VMMInstrument.cpp
  geometry/Config.cpp
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
