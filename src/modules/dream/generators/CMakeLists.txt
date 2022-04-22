# Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# UDP generator for DREAM
#=============================================================================
include_directories(.)
include_directories(..)

#=============================================================================
# Generate DREAM readouts from (.dat) file
#=============================================================================

set(dream_udp_from_dat_SRC
  SimReader.cpp
  essudpgen.cpp
  )
set(dream_udp_from_dat_INC
  SimReader.h
  ../readout/Readout.h
  PacketGenerator.h
  )
create_executable(dream_udp_from_dat)
target_compile_definitions(dream_udp_from_dat
  PUBLIC ESSUDPGEN_DREAM_SIM
  )
set_target_properties(dream_udp_from_dat
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
