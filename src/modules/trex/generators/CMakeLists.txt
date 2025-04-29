# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Generate artificial trex VMM3 readouts
#=============================================================================

set(trex_udp_generated_INC
  ${ESS_MODULE_DIR}/trex/generators/ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  )
set(trex_udp_generated_SRC
  ${ESS_MODULE_DIR}/trex/generators/ReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/trex/generators/udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  )
create_executable(trex_udp_generated)

set_target_properties(trex_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )



#=============================================================================
# Generate artificial LET VMM3 readouts
#=============================================================================

set(let_udp_generated_INC
  ${ESS_MODULE_DIR}/trex/generators/LETReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  )
set(let_udp_generated_SRC
  ${ESS_MODULE_DIR}/trex/generators/LETReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/trex/generators/udpgen_let.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  )
create_executable(let_udp_generated)

set_target_properties(let_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
