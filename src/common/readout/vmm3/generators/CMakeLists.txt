#=============================================================================
# Generate artificial VMM3 readouts
#=============================================================================

set(udpgen_vmm3_freia_INC
  ../VMM3Parser.h
  ../generators/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(udpgen_vmm3_freia_SRC
  udpgen_vmm3_main.cpp
  ../generators/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(udpgen_vmm3_freia)
target_compile_definitions(udpgen_vmm3_freia PUBLIC FREIA_GENERATOR)