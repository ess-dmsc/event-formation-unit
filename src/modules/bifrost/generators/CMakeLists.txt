# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# UDP generator for BIFROST
#=============================================================================
include_directories(.)
include_directories(..)

#=============================================================================
# Generate BIFROST readouts from (.dat) file
#=============================================================================

set(bifrost_udp_from_dat_SRC
  R5560Reader.cpp
  essudpgen.cpp
  )
set(bifrost_udp_from_dat_INC
  R5560Reader.h
  ../readout/Readout.h
  ${ESS_MODULE_DIR}/dream/generators/PacketGenerator.h
  )
create_executable(bifrost_udp_from_dat)
target_compile_definitions(bifrost_udp_from_dat
  PUBLIC ESSUDPGEN_BIFROST_DAT
  )
set_target_properties(bifrost_udp_from_dat
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
