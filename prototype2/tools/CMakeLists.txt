#=============================================================================
# PCAP generator
#=============================================================================

set(udpgen_pcap_SRC
  udpgen_pcap.cpp
  ReaderPcap.cpp
  PcapArgs.cpp
  )
set(udpgen_pcap_INC
  ReaderPcap.h
  PcapArgs.h
  )
set(udpgen_pcap_LIB
  ${PCAP_LIBRARY}
  )
create_executable(udpgen_pcap)

