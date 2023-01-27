# Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file


#=============================================================================
# For replay of readouts in H5 files
#=============================================================================

set(loki_udp_from_h5_INC
  ReaderReadouts.h
  )
set(loki_udp_from_h5_SRC
  ${ESS_SOURCE_DIR}/generators/udpgen_hits/generator.cpp
  ReaderReadouts.cpp
  )
create_executable(loki_udp_from_h5)
target_compile_definitions(loki_udp_from_h5
  PUBLIC GENERATOR_LOKI_READOUTS
  )
set_target_properties(loki_udp_from_h5
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
