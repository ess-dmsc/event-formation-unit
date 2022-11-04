add_subdirectory(generators)
add_subdirectory(test)

include_directories(.)

#=============================================================================
# header and implementation files common to dream.so and DreamBaseTest
#=============================================================================
set(dream_common_inc
  Counters.h
  DreamBase.h
  DreamInstrument.h
  geometry/Config.h
  geometry/CDTGeometry.h
  geometry/Cuboid.h
  geometry/Mantle.h
  geometry/SUMO.h
  readout/DataParser.h
  readout/Readout.h
)

set(dream_common_src
  DreamInstrument.cpp
  DreamBase.cpp
  geometry/CDTGeometry.cpp
  geometry/Config.cpp
  readout/DataParser.cpp
  readout/DataParser.cpp
)

#=============================================================================
# dream detector module
#=============================================================================
set(dream_INC ${dream_common_inc})
set(dream_SRC
  ${dream_common_src}
  ${ESS_SOURCE_DIR}/efu/main.cpp)
set(dream_LIB efu_essreadout)
create_executable(dream)

add_dependencies(dream efu_essreadout)
target_compile_definitions(dream PUBLIC EFU_DREAM)

#=============================================================================
# dream detector module test
#=============================================================================
set(DreamBaseTest_INC
  ${dream_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
)
set(DreamBaseTest_SRC
  ${dream_common_src}
  test/DreamBaseTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
)
create_test_executable(DreamBaseTest)
