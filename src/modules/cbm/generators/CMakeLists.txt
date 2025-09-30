# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Generate artificial CBM readouts - using the new data format
#=============================================================================

set(cbm_udp_generated_INC
  ReadoutGenerator.h
  GeneratorType.h
  ${ESS_COMMON_DIR}/testutils/bitmaps/BitMaps.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/FunctionGenerator.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/LinearGenerator.h
  )
set(cbm_udp_generated_SRC
  ReadoutGenerator.cpp
  udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/bitmaps/BitMaps.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  )
create_executable(cbm_udp_generated)

set_target_properties(cbm_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
