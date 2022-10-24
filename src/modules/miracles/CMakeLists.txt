add_subdirectory(test)

include_directories(.)

#=============================================================================
# miracles detector module and base test
#=============================================================================
set(miracles_common_inc
  Counters.h
  MiraclesBase.h
  MiraclesInstrument.h
  readout/DataParser.h
  geometry/Config.h
  geometry/Geometry.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.h
  )
set(miracles_common_src
  MiraclesBase.cpp
  MiraclesInstrument.cpp
  geometry/Geometry.cpp
  readout/DataParser.cpp
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.cpp
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.cpp
  )
set(miracles_INC ${miracles_common_inc})
set(miracles_SRC Miracles.cpp ${miracles_common_src})
create_module(miracles)




#=============================================================================
# Miracles module test
#=============================================================================
set(MiraclesBaseTest_INC
  ${miracles_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
)
set(MiraclesBaseTest_SRC
  ${miracles_common_src}
  test/MiraclesBaseTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
)
create_test_executable(MiraclesBaseTest)
