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

#
set(FreiaBaseTest_INC
  ${freia_common_inc}
)
set(FreiaBaseTest_SRC
  FreiaBaseTest.cpp
  ${freia_common_src}
)
create_test_executable(FreiaBaseTest)


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
