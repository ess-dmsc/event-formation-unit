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
  )

set(miracles_common_INC
  Counters.h
  MiraclesBase.h
  MiraclesInstrument.h
  readout/DataParser.h
  geometry/Config.h
  geometry/Geometry.h
  )

set(miracles_INC ${miracles_common_INC})
set(miracles_SRC
  ${miracles_common_SRC}
  ${ESS_SOURCE_DIR}/efu/main.cpp
  )
set(miracles_LIB efu_essreadout)
create_executable(miracles)

add_dependencies(miracles efu_essreadout)
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
