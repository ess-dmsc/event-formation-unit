# Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# Generate artificial LoKI readouts
#=============================================================================

set(loki_udp_generated_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_MODULE_DIR}/loki/generators/ReadoutGenerator.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(loki_udp_generated_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/loki/generators/ReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(loki_udp_generated)
target_compile_definitions(loki_udp_generated
  PUBLIC LOKI_GENERATOR
  )
set_target_properties(loki_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )


#=============================================================================
# For replay of readouts in H5 files
#=============================================================================

set(loki_udp_from_h5_INC
  ReaderReadouts.h
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.h
  )
set(loki_udp_from_h5_SRC
  ${ESS_SOURCE_DIR}/generators/udpgen_hits/generator.cpp
  ReaderReadouts.cpp
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.cpp
  )
create_executable(loki_udp_from_h5)
target_compile_definitions(loki_udp_from_h5
  PUBLIC GENERATOR_LOKI_READOUTS
  )
set_target_properties(loki_udp_from_h5
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
