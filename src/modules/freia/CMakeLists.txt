# Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

#=============================================================================
# freia detector module and base test
#=============================================================================
set(freia_common_inc
  FreiaBase.h
  FreiaInstrument.h
  geometry/Config.h
  geometry/FreiaChannelMapping.h
  geometry/AMORChannelMapping.h
  )
set(freia_common_src
  FreiaBase.cpp
  FreiaInstrument.cpp
  geometry/Config.cpp
  )

set(freia_INC ${freia_common_inc})
set(freia_SRC
  ${freia_common_src}
  main.cpp
  )
set(freia_LIB efu_reduction efu_essreadout)
create_executable(freia)

add_dependencies(cspec efu_reduction efu_essreadout)
target_compile_definitions(freia PUBLIC EFU_FREIA)


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
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Config.cpp

)
create_test_executable(FreiaInstrumentTest)
