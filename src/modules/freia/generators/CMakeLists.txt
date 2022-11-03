# Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Generate artificial freia VMM3 readouts
#=============================================================================


set(freia_udp_generated_INC
 ${ESS_SOURCE_DIR}//generators/essudpgen/ReadoutGeneratorBase.h
 ${ESS_MODULE_DIR}/freia/generators/ReadoutGenerator.h
 ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(freia_udp_generated_SRC
  ${ESS_SOURCE_DIR}//generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}//generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/freia/generators/ReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(freia_udp_generated)
target_compile_definitions(freia_udp_generated
  PUBLIC FREIA_GENERATOR
  )
set_target_properties(freia_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
