# Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# UDP generator for DREAM from Geant4 simulations
#=============================================================================
include_directories(.)
include_directories(..)

#=============================================================================
# Generate DREAM readouts from (.dat) file
#=============================================================================

set(dream_udp_generated_g4_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_MODULE_DIR}/dream/generators/g4data/ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  )
set(dream_udp_generated_g4_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/dream/generators/g4data/ReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/dream/generators/g4data/udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  )
create_executable(dream_udp_generated_g4)

set_target_properties(dream_udp_generated_g4
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
