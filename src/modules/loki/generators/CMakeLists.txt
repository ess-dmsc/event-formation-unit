#=============================================================================
# Generate artificial LoKI readouts
#=============================================================================

set(udpgen_loki_INC
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_MODULE_DIR}/loki/generators/ReadoutGenerator.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )

set(udpgen_loki_SRC
  ${ESS_SOURCE_DIR}/generators/essudpgen/udpgen_template.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/loki/generators/ReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )

create_executable(udpgen_loki)
target_compile_definitions(udpgen_loki PUBLIC LOKI_GENERATOR)



#=============================================================================
# For replay of readouts in H5 files
#=============================================================================

set(lokigen_readouts_INC
  ReaderReadouts.h
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.h
  )

set(lokigen_readouts_SRC
  ${ESS_SOURCE_DIR}/generators/udpgen_hits/generator.cpp
  ReaderReadouts.cpp
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.cpp
  )

create_executable(lokigen_readouts)
target_compile_definitions(lokigen_readouts PUBLIC GENERATOR_LOKI_READOUTS)
