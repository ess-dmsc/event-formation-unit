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
  geometry/DreamGeometry.h
  geometry/MagicGeometry.h
  geometry/Cuboid.h
  geometry/Mantle.h
  geometry/PADetector.h
  geometry/SUMO.h
  readout/DataParser.h
  readout/Readout.h
)

set(dream_common_src
  DreamInstrument.cpp
  DreamBase.cpp
  geometry/DreamGeometry.cpp
  geometry/MagicGeometry.cpp
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
  main.cpp)
set(dream_LIB efu_essreadout)
create_executable(dream)

#=============================================================================
# dream detector module test
#=============================================================================
set(DreamBaseTest_INC
  ${dream_common_inc}
)
set(DreamBaseTest_SRC
  ${dream_common_src}
  test/DreamBaseTest.cpp
)
create_test_executable(DreamBaseTest)


#=============================================================================
# magic detector module - identical to dream except instrument name
#=============================================================================
set(magic_INC ${dream_common_inc})
set(magic_SRC
  ${dream_common_src}
  main_magic.cpp)
set(magic_LIB efu_essreadout)
create_executable(magic)
