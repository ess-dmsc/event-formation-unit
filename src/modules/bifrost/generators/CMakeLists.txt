# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# UDP generator for BIFROST
#=============================================================================
include_directories(.)
include_directories(..)

#=============================================================================
# Generate BIFROST readouts from (.dat) file
#=============================================================================

set(bifrost_udp_from_dat_SRC
  DatReader.cpp
  essudpgen.cpp
  )
set(bifrost_udp_from_dat_INC
  DatReader.h
  ${ESS_MODULE_DIR}/caen/readout/Readout.h
  PacketGenerator.h
  )
create_executable(bifrost_udp_from_dat)
set_target_properties(bifrost_udp_from_dat
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
