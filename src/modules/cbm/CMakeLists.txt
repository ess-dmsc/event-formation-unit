# Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

get_filename_component(CBM_CONFIG "${ESS_MODULE_DIR}/cbm/configs/lokimon.json" ABSOLUTE)

#=============================================================================
# cbm detector module and base test
#=============================================================================
set(cbm_common_inc
  CbmBase.h
  CbmInstrument.h
  geometry/Config.h
  geometry/Parser.h
  )


set(cbm_common_src
  CbmBase.cpp
  CbmInstrument.cpp
  geometry/Config.cpp
  geometry/Parser.cpp
  )

#
set(cbm_INC ${cbm_common_inc})
set(cbm_SRC
  ${cbm_common_src}
  main.cpp
  )
set(cbm_LIB efu_essreadout)
create_executable(cbm)


#
set(CbmBaseTest_INC
  ${cbm_common_inc}
)
set(CbmBaseTest_SRC
  test/CbmBaseTest.cpp
  ${cbm_common_src}
)
create_test_executable(CbmBaseTest)
target_compile_definitions(CbmBaseTest PRIVATE CBM_CONFIG="${CBM_CONFIG}")

#
# CBMInstrumentTest Module integration test
#
set(CbmInstrumentTest_INC
  ${cbm_common_inc}
  )
set(CbmInstrumentTest_SRC
  ${cbm_common_src}
  test/CbmInstrumentTest.cpp
)
create_test_executable(CbmInstrumentTest)
target_compile_definitions(CbmInstrumentTest PRIVATE CBM_CONFIG="${CBM_CONFIG}")
