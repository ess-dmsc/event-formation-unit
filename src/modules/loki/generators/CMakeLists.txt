#=============================================================================
# Generate artificial LoKI readouts
#=============================================================================

set(udpgen_loki_INC
  ../readout/DataParser.h
  ../test/ReadoutGenerator.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(udpgen_loki_SRC
  udpgen_loki.cpp
  ../test/ReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(udpgen_loki)

set(udpgen_loki_ii_INC
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/generators/ReadoutGeneratorBase.h
  ${ESS_MODULE_DIR}/loki/generators/LokiReadoutGenerator.h
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.h
  )
set(udpgen_loki_ii_SRC
  ${ESS_SOURCE_DIR}/common/readout/vmm3/generators/udpgen_vmm3_main.cpp
  ${ESS_SOURCE_DIR}/common/readout/vmm3/generators/ReadoutGeneratorBase.cpp
  ${ESS_MODULE_DIR}/loki/generators/LokiReadoutGenerator.cpp
  ${ESS_SOURCE_DIR}/common/testutils/DataFuzzer.cpp
  )
create_executable(udpgen_loki_ii)
target_compile_definitions(udpgen_loki_ii PUBLIC LOKI_GENERATOR)


#=============================================================================
# Generate real LoKI readouts from (.dat) file
# Shares code with DREAM
#=============================================================================

set(udpgen_lokiraw_INC
  RawReader.h
  ${ESS_SOURCE_DIR}/generators/PacketGenerator.h
  ../readout/DataParser.h
  )
set(udpgen_lokiraw_SRC
  RawReader.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen.cpp
  )
create_executable(udpgen_lokiraw)
target_compile_definitions(udpgen_lokiraw PUBLIC ESSUDPGEN_LOKI_RAW)
