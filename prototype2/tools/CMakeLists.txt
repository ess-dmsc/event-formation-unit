#=============================================================================
# PCAP generator
#=============================================================================

find_package(PCAP REQUIRED)
if(PCAP_FOUND)
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
else()
  message(STATUS "*** Unable to compile udpgen_pcap as libpcap was not found.")
endif()
