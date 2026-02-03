# Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Generate artificial BEER readouts - Event 2D data only
#=============================================================================

set(beer_udp_generated_INC
  ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/bitmaps/BitMaps.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/FunctionGenerator.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  # CBM dependencies
  ${ESS_SOURCE_DIR}/modules/cbm/generators/CbmDataGenerator.h
  ${ESS_SOURCE_DIR}/modules/cbm/generators/Event2DDataGenerator.h
  )

set(beer_udp_generated_SRC
  ReadoutGenerator.cpp
  udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/bitmaps/BitMaps.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  # CBM dependencies
  ${ESS_SOURCE_DIR}/modules/cbm/generators/Event2DDataGenerator.cpp
  )

create_executable(beer_udp_generated)

set_target_properties(beer_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
