add_subdirectory(generators)
add_subdirectory(test)

include_directories(.)

#=============================================================================
# header and implementation files common to dream.so and DreamBaseTest
#=============================================================================
set(dream_common_inc
  Counters.h
  readout/DataParser.h
  readout/Readout.h
  DreamBase.h
  DreamInstrument.h
  ../readout/ReadoutParser.h
)

set(dream_common_src
  readout/DataParser.cpp
  Dream.cpp
  DreamInstrument.cpp
  DreamBase.cpp
  readout/DataParser.cpp
  ../readout/ReadoutParser.cpp
)

#=============================================================================
# dream detector module
#=============================================================================
set(dream_INC ${dream_common_inc} )
set(dream_SRC ${dream_common_src} )
create_module(dream)

#=============================================================================
# dream detector module test
#=============================================================================
set(DreamBaseTest_INC
  ${dream_common_inc}
  ${ESS_SOURCE_DIR}/test/TestUDPServer.h
)
set(DreamBaseTest_SRC
  ${dream_common_src}
  test/DreamBaseTest.cpp
  ${ESS_SOURCE_DIR}/test/TestUDPServer.cpp
)
create_test_executable(DreamBaseTest)
