# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Generate artificial CBM readouts - using the new data format
#=============================================================================

set(cbm_udp_generated_INC
  ReadoutGenerator.h
  DataGeneratorFactory.h
  CbmDataGenerator.h
  Event0DDataGenerator.h
  Event2DDataGenerator.h
  IBMDataGenerator.h
  GeneratorType.h
  TimeGeneratorFactory.h
  ${ESS_COMMON_DIR}/testutils/bitmaps/BitMaps.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/FunctionGenerator.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/LinearGenerator.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/FixedValueGenerator.h
  )
set(cbm_udp_generated_SRC
  ReadoutGenerator.cpp
  DataGeneratorFactory.cpp
  Event0DDataGenerator.cpp
  Event2DDataGenerator.cpp
  IBMDataGenerator.cpp
  TimeGeneratorFactory.cpp
  udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/bitmaps/BitMaps.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/FixedValueGenerator.cpp
  )
create_executable(cbm_udp_generated)

set_target_properties(cbm_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
