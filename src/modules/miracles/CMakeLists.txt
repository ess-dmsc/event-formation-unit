add_subdirectory(test)

include_directories(.)

#=============================================================================
# miracles detector module and base test
#=============================================================================
#=============================================================================
# miracles detector module and base test
#=============================================================================

set(miracles_common_SRC
  MiraclesBase.cpp
  MiraclesInstrument.cpp
  geometry/Geometry.cpp
  readout/DataParser.cpp
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.cpp
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.cpp
  )

set(miracles_common_INC
  Counters.h
  MiraclesBase.h
  MiraclesInstrument.h
  readout/DataParser.h
  geometry/Config.h
  geometry/Geometry.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.h
  )

set(miracles_SRC ${EFUCOMMON_SRC} ${miracles_common_SRC})
set(miracles_INC ${EFUCOMMON_INC} ${miracles_common_INC})
create_executable(miracles)
target_compile_definitions(miracles PUBLIC EFU_MIRACLES)



#=============================================================================
# Miracles module test
#=============================================================================
set(MiraclesBaseTest_INC
  ${miracles_common_INC}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
)
set(MiraclesBaseTest_SRC
  ${miracles_common_SRC}
  test/MiraclesBaseTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
)
create_test_executable(MiraclesBaseTest)
