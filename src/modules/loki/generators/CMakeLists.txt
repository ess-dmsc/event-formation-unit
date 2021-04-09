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
# Shares code with DREAM
#=============================================================================

set(udpgen_lokiraw_INC
  RawReader.h
  ../../generators/PacketGenerator.h
  ../readout/DataParser.h
  )
set(udpgen_lokiraw_SRC
  RawReader.cpp
  ../../generators/essudpgen.cpp
  )
create_executable(udpgen_lokiraw)
target_compile_definitions(udpgen_lokiraw PUBLIC ESSUDPGEN_LOKI_RAW)
