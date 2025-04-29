# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Generate artificial NMX VMM3 readouts
#=============================================================================

set(nmx_udp_generated_INC
  ${ESS_MODULE_DIR}/nmx/generators/ReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  )
set(nmx_udp_generated_SRC
  ${ESS_MODULE_DIR}/nmx/generators/ReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/nmx/generators/udpgen.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  )
create_executable(nmx_udp_generated)

set_target_properties(nmx_udp_generated
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )


#=============================================================================
# Generate artificial NMX VMM3 readouts - Smiling face pattern
#=============================================================================
set(nmx_udp_generated_smile_INC
  ${ESS_MODULE_DIR}/nmx/generators/SmileReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  )
set(nmx_udp_generated_smile_SRC
  ${ESS_MODULE_DIR}/nmx/generators/SmileReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/nmx/generators/udpgen_smile.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  )
create_executable(nmx_udp_generated_smile)

set_target_properties(nmx_udp_generated_smile
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )


#=============================================================================
# Generate artificial NMX VMM3 readouts - TBD
#=============================================================================
set(nmx_udp_generated_multihit_INC
  ${ESS_MODULE_DIR}/nmx/generators/MultiHitReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  )
set(nmx_udp_generated_multihit_SRC
  ${ESS_MODULE_DIR}/nmx/generators/MultiHitReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/nmx/generators/udpgen_multihit.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  )
create_executable(nmx_udp_generated_multihit)

set_target_properties(nmx_udp_generated_multihit
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )

#=============================================================================
# Generate artificial NMX VMM3 readouts - track pattern
#=============================================================================
set(nmx_udp_generated_track_INC
  ${ESS_MODULE_DIR}/nmx/generators/TrackReadoutGenerator.h
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.h
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.h
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.h
  )
set(nmx_udp_generated_track_SRC
  ${ESS_MODULE_DIR}/nmx/generators/TrackReadoutGenerator.cpp
  ${ESS_MODULE_DIR}/nmx/generators/udpgen_track.cpp
  ${ESS_COMMON_DIR}/testutils/DataFuzzer.cpp
  ${ESS_SOURCE_DIR}/generators/essudpgen/ReadoutGeneratorBase.cpp
  ${ESS_SOURCE_DIR}/generators/functiongenerators/DistributionGenerator.cpp
  )
create_executable(nmx_udp_generated_track)

set_target_properties(nmx_udp_generated_track
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
