add_subdirectory(generators)

include_directories(.)

#=============================================================================
# dream detector module
#=============================================================================

set(dream_INC
  Counters.h
  readout/DataParser.h
  readout/Readout.h
  DreamBase.h
  DreamInstrument.h
  ../readout/ReadoutParser.h
  )
set(dream_SRC
  readout/DataParser.cpp
  Dream.cpp
  DreamInstrument.cpp
  DreamBase.cpp
  readout/DataParser.cpp
  ../readout/ReadoutParser.cpp
  )
create_module(dream)
