# Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file

include_directories(.)

#=============================================================================
# timepix3 detector module and base test
#=============================================================================
set(timepix3_common_inc
  Timepix3Base.h
  Timepix3Instrument.h
  geometry/Config.h
  geometry/Timepix3Geometry.h
  readout/DataParser.h
  )
set(timepix3_common_src
  Timepix3Base.cpp
  Timepix3Instrument.cpp
  geometry/Config.cpp
  geometry/Timepix3Geometry.cpp
  readout/DataParser.cpp
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

set(Timepix3BaseTest_INC
  ${timepix3_common_inc}
)
set(Timepix3BaseTest_SRC
  ${timepix3_common_src}
  test/Timepix3BaseTest.cpp
)
create_test_executable(Timepix3BaseTest)

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


