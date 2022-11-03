# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Generate artificial CSPEC VMM3 readouts
#=============================================================================

set(cspec_udp_generated_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_MODULE_DIR}/cspec/generators/ReadoutGenerator.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(cspec_udp_generated_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/cspec/generators/ReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(cspec_udp_generated)
target_compile_definitions(cspec_udp_generated
  PUBLIC CSPEC_GENERATOR
  )
set_target_properties(cspec_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )


#=============================================================================
# Generate artificial LET VMM3 readouts
#=============================================================================

set(let_udp_generated_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_MODULE_DIR}/cspec/generators/LETReadoutGenerator.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(let_udp_generated_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/cspec/generators/LETReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(let_udp_generated)
target_compile_definitions(let_udp_generated
  PUBLIC LET_GENERATOR
  )
set_target_properties(let_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
