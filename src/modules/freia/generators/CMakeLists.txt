# Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file

#===============================================================================
# Generate artificial freia VMM3 readouts
#===============================================================================

set(freia_udp_generated_INC
  ${ESS_MODULE_DIR}/freia/generators/ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  )
set(freia_udp_generated_SRC
  ${ESS_MODULE_DIR}/freia/generators/ReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/freia/generators/udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  )

create_executable(freia_udp_generated)

set_target_properties(freia_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )


#===============================================================================
# Generate artificial multi-blade VMM3 readouts
#===============================================================================

set(mbvm3_udp_generated_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  ${ESS_MODULE_DIR}/freia/generators/MultiBladeGenerator.h
  ${ESS_COMMON_DIR}/testutils/bitmaps/BitMaps.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  )

set(mbvm3_udp_generated_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  ${ESS_MODULE_DIR}/freia/generators/MultiBladeGenerator.cpp
  ${ESS_COMMON_DIR}/testutils/bitmaps/BitMaps.cpp
  ${ESS_MODULE_DIR}/freia/generators/mb_udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  )
create_executable(mbvm3_udp_generated)

set_target_properties(mbvm3_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
