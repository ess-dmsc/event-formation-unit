#=============================================================================
# Generate artificial CSPEC VMM3 readouts
#=============================================================================

set(udpgen_vmm3_cspec_INC
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/generators/ReadoutGeneratorBase.h
  ${ESS_MODULE_DIR}/cspec/generators/ReadoutGenerator.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(udpgen_vmm3_cspec_SRC
  ${ESS_SOURCE_DIR}/common/readout/vmm3/generators/udpgen_vmm3_main.cpp
  ${ESS_SOURCE_DIR}/common/readout/vmm3/generators/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/cspec/generators/ReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(udpgen_vmm3_cspec)
target_compile_definitions(udpgen_vmm3_cspec PUBLIC CSPEC_GENERATOR)