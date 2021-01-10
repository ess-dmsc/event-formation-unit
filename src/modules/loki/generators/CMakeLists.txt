#=============================================================================
# Generate artificial LoKI readouts
#=============================================================================

set(udpgen_loki_INC
  ../readout/DataParser.h
  ../test/ReadoutGenerator.h)
set(udpgen_loki_SRC
  udpgen_loki.cpp
  ../test/ReadoutGenerator.cpp)
create_executable(udpgen_loki)


#=============================================================================
# Generate real LoKI readouts from (.dat) file
#=============================================================================

set(udpgen_lokiraw_INC
  RawReader.h
  LokiPacketGen.h
  ../readout/DataParser.h
  )
set(udpgen_lokiraw_SRC
  RawReader.cpp
  udpgen_lokiraw.cpp
  )
create_executable(udpgen_lokiraw)
