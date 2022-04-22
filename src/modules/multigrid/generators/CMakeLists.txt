# Copyright (C) 2019-2022 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# Raw event generator from readouts(h5)
#=============================================================================

set(obsolete_mggen_readouts_SRC
  ${ESS_SOURCE_DIR}/generators/udpgen_hits/generator.cpp
  ../mesytec/Readout.cpp
  ReaderReadouts.cpp
  )
set(obsolete_mggen_readouts_INC
  ../mesytec/Readout.h
  ReaderReadouts.h
  )

create_executable(obsolete_mggen_readouts)
target_compile_definitions(obsolete_mggen_readouts
  PUBLIC GENERATOR_MULTIGRID_READOUTS
  )
set_target_properties(obsolete_mggen_readouts
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
