# Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

#=============================================================================
# trex detector module and base test
#=============================================================================
set(trex_common_inc
  TREXBase.h
  TREXInstrument.h
  geometry/Config.h
  geometry/TREXGeometry.h
  geometry/LETGeometry.h
  geometry/Geometry.h
  )
set(trex_common_src
  TREXBase.cpp
  TREXInstrument.cpp
  geometry/Config.cpp
  geometry/TREXGeometry.cpp
  geometry/LETGeometry.cpp
  )

set(trex_INC
  ${trex_common_inc}
  )
set(trex_SRC
  main.cpp
  ${trex_common_src}
  )
set(trex_LIB
  efu_reduction
  efu_essreadout
  )
create_executable(trex)

set(TREXBaseTest_INC
  ${trex_common_inc}
)
set(TREXBaseTest_SRC
  test/TREXBaseTest.cpp
  ${trex_common_src}
)
create_test_executable(TREXBaseTest)

#
# TREX InstrumentTest Module integration test
#
set(TREXInstrumentTest_INC
  ${trex_common_inc}
  )
set(TREXInstrumentTest_SRC
  ${trex_common_src}
  test/TREXInstrumentTest.cpp
)
create_test_executable(TREXInstrumentTest)
