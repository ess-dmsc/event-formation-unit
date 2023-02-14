# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# UDP generator for MIRACLES
#=============================================================================
include_directories(.)
include_directories(..)

#=============================================================================
# Generate MIRACLES readouts from (.dat) file
#=============================================================================

set(miracles_udp_from_dat_SRC
  DatReader.cpp
  essudpgen.cpp
  )
set(miracles_udp_from_dat_INC
  DatReader.h
  ${ESS_MODULE_DIR}/caen/readout/Readout.h
  PacketGenerator.h
  )
create_executable(miracles_udp_from_dat)
set_target_properties(miracles_udp_from_dat
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
