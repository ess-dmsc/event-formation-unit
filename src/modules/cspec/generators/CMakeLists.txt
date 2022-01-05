#=============================================================================
# Generate artificial CSPEC readouts
#=============================================================================

set(udpgen_cspec_INC
  ../readout/DataParser.h
  ../generators/ReadoutGenerator.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(udpgen_cspec_SRC
  udpgen_cspec.cpp
  ../generators/ReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(udpgen_cspec)

