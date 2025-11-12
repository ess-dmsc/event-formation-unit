# Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file

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
  )
set(nmx_common_src
  NMXBase.cpp
  NMXInstrument.cpp
  geometry/Config.cpp
  geometry/NMXGeometry.cpp
  )

set(nmx_INC ${nmx_common_inc})
set(nmx_SRC
  ${nmx_common_src}
  main.cpp
  )
set(nmx_LIB efu_reduction efu_essreadout)
create_executable(nmx)

set(NMXBaseTest_INC
  ${nmx_common_inc}
)
set(NMXBaseTest_SRC
  test/NMXBaseTest.cpp
  ${nmx_common_src}
)
create_test_executable(NMXBaseTest)


# NMX InstrumentTest Module integration test

set(NMXInstrumentTest_INC
  ${nmx_common_inc}
  )
set(NMXInstrumentTest_SRC
  ${nmx_common_src}
  test/NMXInstrumentTest.cpp
)

set(NMXInstrumentTest_LIB efu_essreadout)
create_test_executable(NMXInstrumentTest)
