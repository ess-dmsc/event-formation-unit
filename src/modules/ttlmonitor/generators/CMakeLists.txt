# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# Generate artificial TTLMonitor readouts
#=============================================================================

set(ttlmon_udp_generated_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_MODULE_DIR}/ttlmonitor/generators/ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  )
set(ttlmon_udp_generated_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/ttlmonitor/generators/ReadoutGenerator.cpp
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

# using the new data format
set(ttlmon_udp_generated_new_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_MODULE_DIR}/ttlmonitor/generators/ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  )
set(ttlmon_udp_generated_new_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/ttlmonitor/generators/ReadoutGeneratorNew.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  )
create_executable(ttlmon_udp_generated_new)
target_compile_definitions(ttlmon_udp_generated_new
  PUBLIC TTLMON_GENERATOR_NEW
  )
set_target_properties(ttlmon_udp_generated_new
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
