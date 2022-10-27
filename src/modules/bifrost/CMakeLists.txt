add_subdirectory(generators)
add_subdirectory(test)

include_directories(.)

#=============================================================================
# bifrost detector module and base test
#=============================================================================
set(bifrost_common_inc
  Counters.h
  BifrostBase.h
  BifrostInstrument.h
  readout/DataParser.h
  geometry/Config.h
  geometry/Geometry.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.h
  )
set(bifrost_common_src
  BifrostBase.cpp
  BifrostInstrument.cpp
  geometry/Geometry.cpp
  readout/DataParser.cpp
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.cpp
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.cpp
  )
set(bifrost_INC ${bifrost_common_inc})
set(bifrost_SRC ${bifrost_common_src})
create_module(bifrost)




#=============================================================================
# Bifrost module test
#=============================================================================
set(BifrostBaseTest_INC
  ${bifrost_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
)
set(BifrostBaseTest_SRC
  ${bifrost_common_src}
  test/BifrostBaseTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
)
create_test_executable(BifrostBaseTest)
