#=============================================================================
# Generate artificial freia VMM3 readouts
#=============================================================================


set(udpgen_vmm3_freia_INC
 ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.h
 ${ESS_SOURCE_DIR}//generators/essudpgen/ReadoutGeneratorBase.h
 ${ESS_MODULE_DIR}/freia/generators/ReadoutGenerator.h
 ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(udpgen_vmm3_freia_SRC
  ${ESS_SOURCE_DIR}//generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}//generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/freia/generators/ReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(udpgen_vmm3_freia)
target_compile_definitions(udpgen_vmm3_freia PUBLIC FREIA_GENERATOR)
