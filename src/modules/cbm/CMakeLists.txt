# Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(generators)
include_directories(.)

#=============================================================================
# cbm detector module and base test
#=============================================================================
set(cbm_common_inc
  CbmBase.h
  CbmInstrument.h
  CbmTypes.h
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

#============================================================================
# CBMBaseTest Module integration test
#============================================================================
set(CbmBaseTest_INC
  ${cbm_common_inc}
)
set(CbmBaseTest_SRC
  ${cbm_common_src}
  test/CbmBaseTest.cpp
)
create_test_executable(CbmBaseTest)

#============================================================================
# CBMInstrumentTest Module integration test
#============================================================================
set(CbmInstrumentTest_INC
  ${cbm_common_inc}
)
set(CbmInstrumentTest_SRC
  ${cbm_common_src}
  test/CbmInstrumentTest.cpp
)
create_test_executable(CbmInstrumentTest)

#============================================================================
# CBMParserTest Module integration test
#============================================================================
set(CbmParserTest_INC
  ${cbm_common_inc}
)
set(CbmParserTest_SRC
  ${cbm_common_src}
  test/ParserTest.cpp)
create_test_executable(CbmParserTest)

#============================================================================
# CBMConfigTest Module integration test
#============================================================================
set(CbmConfigTest_INC
  ${cbm_common_inc}  
)
set(CbmConfigTest_SRC
  ${cbm_common_src}
  test/ConfigTest.cpp)
create_test_executable(CbmConfigTest)