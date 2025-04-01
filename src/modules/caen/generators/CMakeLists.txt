# Copyright (C) 2020 - 2024 European Spallation Source, ERIC. See LICENSE file

  #=============================================================================
  # Generate CAEN readouts
  #=============================================================================

  set(caen_udp_generated_INC
    ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
    ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
    ${ESS_MODULE_DIR}/caen/generators/ReadoutGenerator.h
    ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
    )
  set(caen_udp_generated_SRC
    ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
    ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
    ${ESS_MODULE_DIR}/caen/generators/ReadoutGenerator.cpp
    ${ESS_MODULE_DIR}/caen/generators/udpgen.cpp
    ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
    )
  create_executable(caen_udp_generated)

  set_target_properties(caen_udp_generated
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
    )
