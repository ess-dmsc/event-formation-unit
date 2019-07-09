#=============================================================================
# PCAP generator
#=============================================================================

set(udpgen_jalousie_SRC
  udpgen_jalousie.cpp
  ReaderJalousie.cpp
  )
set(udpgen_jalousie_INC
  ReaderJalousie.h
  )

create_executable(udpgen_jalousie)
