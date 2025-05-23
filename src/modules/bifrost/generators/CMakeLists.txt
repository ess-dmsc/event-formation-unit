# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# UDP generator for BIFROST
#=============================================================================
include_directories(.)
include_directories(..)

#=============================================================================
# Generate BIFROST readouts from (.dat) file
#=============================================================================

set(bifrost_udp_generated_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  ${ESS_MODULE_DIR}/bifrost/generators/ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  )
set(bifrost_udp_generated_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  ${ESS_MODULE_DIR}/bifrost/generators/ReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/bifrost/generators/udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  )
create_executable(bifrost_udp_generated)

set_target_properties(bifrost_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
