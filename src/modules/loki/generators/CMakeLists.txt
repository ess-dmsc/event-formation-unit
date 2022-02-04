#=============================================================================
# Generate artificial LoKI readouts
#=============================================================================

set(udpgen_loki_INC
  ${ESS_SOURCE_DIR}//generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_MODULE_DIR}/loki/generators/LokiReadoutGenerator.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(udpgen_loki_SRC
  ${ESS_SOURCE_DIR}//generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}//generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/loki/generators/LokiReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(udpgen_loki)
target_compile_definitions(udpgen_loki PUBLIC LOKI_GENERATOR)
