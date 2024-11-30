# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

get_filename_component(ESTIA_FULL "${ESS_MODULE_DIR}/estia/configs/estia.json" ABSOLUTE)

#=============================================================================
# estia detector module and base test
#=============================================================================
set(estia_common_inc
  EstiaBase.h
  EstiaInstrument.h
  geometry/Config.h
  geometry/EstiaChannelMapping.h
  )
set(estia_common_src
  EstiaBase.cpp
  EstiaInstrument.cpp
  geometry/Config.cpp
  )

set(estia_INC ${estia_common_inc})
set(estia_SRC
  ${estia_common_src}
  main.cpp
  )
set(estia_LIB efu_reduction efu_essreadout)
create_executable(estia)



##
## EstiaBaseTest Module integration test
##
set(EstiaBaseTest_INC
  ${estia_common_inc}
)
set(EstiaBaseTest_SRC
  test/EstiaBaseTest.cpp
  ${estia_common_src}
)
create_test_executable(EstiaBaseTest)
target_compile_definitions(EstiaBaseTest PRIVATE ESTIA_FULL="${ESTIA_FULL}")


##
## EstiaInstrumentTest Module integration test
##
set(EstiaInstrumentTest_INC
  ${estia_common_inc}
  )
set(EstiaInstrumentTest_SRC
  ${estia_common_src}
  test/EstiaInstrumentTest.cpp
)
create_test_executable(EstiaInstrumentTest)
target_compile_definitions(EstiaInstrumentTest PRIVATE ESTIA_FULL="${ESTIA_FULL}")
