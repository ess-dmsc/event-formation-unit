# Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

get_filename_component(FREIA_FULL "${ESS_MODULE_DIR}/freia/configs/freia.json" ABSOLUTE)

#=============================================================================
# freia detector module and base test
#=============================================================================
set(freia_common_inc
  FreiaBase.h
  FreiaInstrument.h
  geometry/Config.h
  geometry/GeometryBase.h
  geometry/FreiaGeometry.h
  geometry/EstiaGeometry.h
  geometry/AmorGeometry.h
  )

set(freia_common_src
  FreiaBase.cpp
  FreiaInstrument.cpp
  geometry/Config.cpp
  geometry/GeometryBase.cpp
  )

#=============================================================================
# freia detector binary
#=============================================================================
set(freia_INC ${freia_common_inc})
set(freia_SRC
  ${freia_common_src}
  main_freia.cpp
  )
set(freia_LIB efu_reduction efu_essreadout)
create_executable(freia)

#=============================================================================
# estia detector binary
#=============================================================================
set(estia_INC ${freia_common_inc})
set(estia_SRC
  ${freia_common_src}
  main_estia.cpp
  )
set(estia_LIB efu_reduction efu_essreadout)
create_executable(estia)

#=============================================================================
# tblmb detector binary
#=============================================================================
set(tblmb_INC ${freia_common_inc})
set(tblmb_SRC
  ${freia_common_src}
  main_tblmb.cpp
  )
set(tblmb_LIB efu_reduction efu_essreadout)
create_executable(tblmb)


##
## FreiaiBaseTest Module integration test
##
set(FreiaBaseTest_INC
  ${freia_common_inc}
)
set(FreiaBaseTest_SRC
  test/FreiaBaseTest.cpp
  ${freia_common_src}
)
create_test_executable(FreiaBaseTest)
target_compile_definitions(FreiaBaseTest PRIVATE FREIA_FULL="${FREIA_FULL}")


##
## FreiaiInstrumentTest Module integration test
##
set(FreiaInstrumentTest_INC
  ${freia_common_inc}
  )
set(FreiaInstrumentTest_SRC
  ${freia_common_src}
  test/FreiaInstrumentTest.cpp
)
create_test_executable(FreiaInstrumentTest)
target_compile_definitions(FreiaInstrumentTest PRIVATE FREIA_FULL="${FREIA_FULL}")
