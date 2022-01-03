#=============================================================================
# Generate artificial VMM3 readouts
#=============================================================================

set(udpgen_vmm3_freia_INC
  ../VMM3Parser.h
  ../test/ReadoutGenerator.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(udpgen_vmm3_freia_SRC
  udpgen_vmm3_freia.cpp
  ../test/ReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(udpgen_vmm3_freia)
