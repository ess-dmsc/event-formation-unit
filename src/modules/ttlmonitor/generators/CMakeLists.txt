#=============================================================================
# Generate artificial TTLMonitor readouts
#=============================================================================

set(udpgen_ttlmon_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_MODULE_DIR}/ttlmonitor/generators/ReadoutGenerator.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(udpgen_ttlmon_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/ttlmonitor/generators/ReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(udpgen_ttlmon)
target_compile_definitions(udpgen_ttlmon PUBLIC TTLMON_GENERATOR)
