#=============================================================================
# PCAP generator
#=============================================================================

set(udpgen_jalousie2_SRC
  udpgen_jalousie.cpp
  ReaderJalousie.cpp
  )
set(udpgen_jalousie2_INC
  ReaderJalousie.h
  )

create_executable(udpgen_jalousie2)
