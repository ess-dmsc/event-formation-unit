# Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file


#=============================================================================
# Generate artificial LoKI readouts
#=============================================================================

set(loki_udp_generated_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  ${ESS_MODULE_DIR}/loki/generators/ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  )
set(loki_udp_generated_SRC
  ${ESS_MODULE_DIR}/loki/generators/ReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/loki/generators/udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  )
create_executable(loki_udp_generated)

set_target_properties(loki_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
