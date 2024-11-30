# Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Generate artificial estia VMM3 readouts
#=============================================================================


set(estia_udp_generated_INC
  ${ESS_MODULE_DIR}/estia/generators/ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  )
set(estia_udp_generated_SRC
  ${ESS_MODULE_DIR}/estia/generators/ReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/estia/generators/udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  )

create_executable(estia_udp_generated)

set_target_properties(estia_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
