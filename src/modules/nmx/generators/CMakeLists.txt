# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Generate artificial NMX VMM3 readouts
#=============================================================================

set(nmx_udp_generated_INC
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_MODULE_DIR}/nmx/generators/ReadoutGenerator.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(nmx_udp_generated_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/nmx/generators/ReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(nmx_udp_generated)
target_compile_definitions(nmx_udp_generated
  PUBLIC NMX_GENERATOR
  )
set_target_properties(nmx_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
