#=============================================================================
# PCAP generator
#=============================================================================
include_directories(.)
include_directories(..)


#=============================================================================
# Generate real LoKI readouts from (.dat) file
# Shares code with LoKI
#=============================================================================
set(udpgen_dreamsim_SRC
  SimReader.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen.cpp
  )
set(udpgen_dreamsim_INC
  SimReader.h
  ../readout/Readout.h
  ${ESS_SOURCE_DIR}/generators/PacketGenerator.h
  )
create_executable(udpgen_dreamsim)
target_compile_definitions(udpgen_dreamsim PUBLIC ESSUDPGEN_DREAM_SIM)
