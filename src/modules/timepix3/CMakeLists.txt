# Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file

include_directories(.)

get_filename_component(TIMEPIX_CONFIG "${ESS_MODULE_DIR}/timepix3/configs/timepix3.json" ABSOLUTE)

#=============================================================================
# timepix3 detector module and base test
#=============================================================================
set(timepix3_common_inc
  Timepix3Base.h
  geometry/Config.h
  geometry/Timepix3Geometry.h
  readout/DataEventTypes.h
  readout/DataEventListener.h
  readout/DataEventManager.h
  readout/DataParser.h
  Timepix3Instrument.h
  )
set(timepix3_common_src
  Timepix3Base.cpp
  readout/DataEventListener.cpp
  readout/DataEventManager.cpp
  readout/DataEventTypes.cpp
  readout/DataParser.cpp
  Timepix3Instrument.cpp
  geometry/Config.cpp
  geometry/Timepix3Geometry.cpp
  )

add_library(timepix3_common STATIC
  ${timepix3_common_src}
  ${timepix3_common_inc}
  )

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(timepix3_common "-Wl,--no-as-needed")
endif()

set(timepix3_INC ${timepix3_common_inc})
set(timepix3_SRC
  ${timepix3_common_src}
  main.cpp
  )
set(timepix3_LIB efu_reduction efu_essreadout)
create_executable(timepix3)


##
## Timepix3InstrumentTest Module integration test
##
set(Timepix3InstrumentTest_INC
  ${timepix3_common_inc}
  )
set(Timepix3InstrumentTest_SRC
  ${timepix3_common_src}
  test/Timepix3InstrumentTest.cpp
)
create_test_executable(Timepix3InstrumentTest)
target_compile_definitions(Timepix3InstrumentTest PRIVATE TIMEPIX_CONFIG="${TIMEPIX_CONFIG}")

set(Timepix3BaseTest_INC
  ${timepix3_common_inc}
)
set(Timepix3BaseTest_SRC
  ${timepix3_common_src}
  test/Timepix3BaseTest.cpp
)
create_test_executable(Timepix3BaseTest)
target_compile_definitions(Timepix3BaseTest PRIVATE TIMEPIX_CONFIG="${TIMEPIX_CONFIG}")

set(Timepix3ParserTest_INC
  ${timepix3_common_inc}
)
set(Timepix3ParserTest_SRC
  ${timepix3_common_src}
  test/Timepix3ParserTest.cpp
)
create_test_executable(Timepix3ParserTest)

set(Timepix3GeometryTest_INC
  ${timepix3_common_inc}
)
set(Timepix3GeometryTest_SRC
  ${timepix3_common_src}
  test/Timepix3GeometryTest.cpp
)
create_test_executable(Timepix3GeometryTest)


