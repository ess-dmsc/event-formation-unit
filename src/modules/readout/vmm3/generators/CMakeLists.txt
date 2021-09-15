#=============================================================================
# Generate artificial VMM3 readouts
#=============================================================================

set(udpgen_vmm3_freia_INC
  ../VMM3Parser.h
  ../test/ReadoutGenerator.h
  ${ESS_MODULE_DIR}/generators/DataFuzzer.h
  )
set(udpgen_vmm3_freia_SRC
  udpgen_vmm3_freia.cpp
  ../test/ReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/generators/DataFuzzer.cpp
  )
create_executable(udpgen_vmm3_freia)
