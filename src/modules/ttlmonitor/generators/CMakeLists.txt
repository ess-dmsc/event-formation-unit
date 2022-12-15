# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# Generate artificial TTLMonitor readouts
#=============================================================================

set(ttlmon_vmm_udp_generated_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  )
set(ttlmon_vmm_udp_generated_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ReadoutGeneratorVMM.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  )
create_executable(ttlmon_vmm_udp_generated)
target_compile_definitions(ttlmon_vmm_udp_generated
  PUBLIC TTLMON_GENERATOR_VMM
  )
set_target_properties(ttlmon_vmm_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )

# using the new data format
set(ttlmon_udp_generated_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  )
set(ttlmon_udp_generated_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ReadoutGenerator.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  )
create_executable(ttlmon_udp_generated)
target_compile_definitions(ttlmon_udp_generated
  PUBLIC TTLMON_GENERATOR
  )
set_target_properties(ttlmon_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
