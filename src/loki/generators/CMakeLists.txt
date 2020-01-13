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
