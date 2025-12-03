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
  geometry/HeimdalGeometry.h
  geometry/MagicGeometry.h
  geometry/Cuboid.h
  geometry/HeimdalMantle.h
  geometry/DreamMantle.h
  geometry/PADetector.h
  geometry/SUMO.h
  readout/DataParser.h
  readout/Readout.h
)

set(dream_common_src
  geometry/DreamGeometry.cpp
  geometry/HeimdalGeometry.cpp
  geometry/MagicGeometry.cpp
  geometry/Config.cpp
  geometry/SUMO.cpp
  geometry/DreamMantle.cpp
  geometry/HeimdalMantle.cpp
  geometry/Cuboid.cpp
  geometry/PADetector.cpp
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

#=============================================================================
# heimdal detector module - identical to dream except instrument name
#=============================================================================
set(heimdal_INC ${dream_common_inc})
set(heimdal_SRC
  ${dream_common_src}
  main_heimdal.cpp)
set(heimdal_LIB efu_essreadout)
create_executable(heimdal)
