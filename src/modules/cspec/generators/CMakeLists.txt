# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Generate artificial CSPEC VMM3 readouts
#=============================================================================

set(cspec_udp_generated_INC
  ${ESS_MODULE_DIR}/cspec/generators/ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  )
set(cspec_udp_generated_SRC
  ${ESS_MODULE_DIR}/cspec/generators/ReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/cspec/generators/udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  )
create_executable(cspec_udp_generated)

set_target_properties(cspec_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )



#=============================================================================
# Generate artificial LET VMM3 readouts
#=============================================================================

set(let_udp_generated_INC
  ${ESS_MODULE_DIR}/cspec/generators/LETReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  )
set(let_udp_generated_SRC
  ${ESS_MODULE_DIR}/cspec/generators/LETReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/cspec/generators/udpgen_let.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  )
create_executable(let_udp_generated)

set_target_properties(let_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
