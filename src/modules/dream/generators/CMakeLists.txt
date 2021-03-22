#=============================================================================
# PCAP generator
#=============================================================================
include_directories(.)
include_directories(..)

#
set(udpgen_dreamsim_SRC
  SimReader.cpp
  udpgen_dreamsim.cpp
  )
set(udpgen_dreamsim_INC
  SimReader.h
  ../readout/Readout.h
  ../../generators/PacketGenerator.h
  )
create_executable(udpgen_dreamsim)
