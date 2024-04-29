# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# UDP generator for DREAM
#=============================================================================
include_directories(.)
include_directories(..)

#=============================================================================
# Generate DREAM readouts
#=============================================================================

set(dream_udp_generated_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/CustomDistribution.h
  ${ESS_MODULE_DIR}/dream/generators/ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  )
set(dream_udp_generated_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/CustomDistribution.cpp
  ${ESS_MODULE_DIR}/dream/generators/ReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/dream/generators/udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  )
create_executable(dream_udp_generated)

set_target_properties(dream_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
