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
  readout/DataParser.h
  readout/Readout.h
  ../readout/common/Parser.h
)

set(dream_common_src
  Dream.cpp
  DreamInstrument.cpp
  DreamBase.cpp
  geometry/Config.cpp
  readout/DataParser.cpp
  readout/DataParser.cpp
  ../readout/common/Parser.cpp
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
  ${ESS_SOURCE_DIR}/test/SaveBuffer.h
  ${ESS_SOURCE_DIR}/test/TestUDPServer.h
)
set(DreamBaseTest_SRC
  ${dream_common_src}
  test/DreamBaseTest.cpp
  ${ESS_SOURCE_DIR}/test/SaveBuffer.cpp
  ${ESS_SOURCE_DIR}/test/TestUDPServer.cpp
)
create_test_executable(DreamBaseTest)
