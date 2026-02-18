# Copyright (C) 2025 - 2026 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(generators)
include_directories(.)
include_directories(../cbm)

#=============================================================================
# beer detector module and base test
#=============================================================================
# Beer extends CBM, so we need to compile CBM sources as part of beer
set(cbm_src_dir ../cbm)

set(beer_common_inc
  BeerBase.h
  readout/Parser.h
  )

set(beer_common_src
  BeerBase.cpp
  readout/Parser.cpp
  ${cbm_src_dir}/CbmBase.cpp
  ${cbm_src_dir}/CbmInstrument.cpp
  ${cbm_src_dir}/geometry/Config.cpp
  ${cbm_src_dir}/readout/Parser.cpp
  )

#
set(beer_INC ${beer_common_inc})
set(beer_SRC
  ${beer_common_src}
  main.cpp
  )
set(beer_LIB efu_essreadout)
create_executable(beer)

#============================================================================
# BeerBaseTest Module integration test
#============================================================================
set(BeerBaseTest_INC
  ${beer_common_inc}
)
set(BeerBaseTest_SRC
  ${beer_common_src}
  test/BeerBaseTest.cpp
)
create_test_executable(BeerBaseTest)

#============================================================================
# BeerConfigTest - Beer configuration unit test
#============================================================================
set(BeerConfigTest_INC
  geometry/Config.h
  ${cbm_src_dir}/geometry/Config.h
)
set(BeerConfigTest_SRC
  ${cbm_src_dir}/geometry/Config.cpp
  test/ConfigTest.cpp
)
create_test_executable(BeerConfigTest)

#============================================================================
# BeerParserTest - Beer parser unit test
#============================================================================
set(BeerParserTest_INC
  readout/Parser.h
  ${cbm_src_dir}/readout/Parser.h
)
set(BeerParserTest_SRC
  readout/Parser.cpp
  ${cbm_src_dir}/readout/Parser.cpp
  test/ParserTest.cpp
)
create_test_executable(BeerParserTest)
