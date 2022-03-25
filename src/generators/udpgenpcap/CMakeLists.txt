# Copyright (C) 2021-2022 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# PCAP generator
#=============================================================================

set(udpgen_pcap_SRC
  udpgen_pcap.cpp
  ReaderPcap.cpp
  )
set(udpgen_pcap_INC
  ReaderPcap.h
  )
set(udpgen_pcap_LIB
  ${PCAP_LIBRARY}
  )
create_executable(udpgen_pcap)
set_target_properties(udpgen_pcap
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
