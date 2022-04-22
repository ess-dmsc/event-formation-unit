# Copyright (C) 2019-2022 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# NMX Readout generator (h5)
#=============================================================================

set(obsolete_nmxgen_readouts_SRC
  ${ESS_SOURCE_DIR}/generators/udpgen_hits/generator.cpp
  ReaderReadouts.cpp
  )
set(obsolete_nmxgen_readouts_INC
  ReaderReadouts.h
  )
create_executable(obsolete_nmxgen_readouts)
target_compile_definitions(obsolete_nmxgen_readouts
  PUBLIC GENERATOR_GDGEM_READOUTS
  )
set_target_properties(obsolete_nmxgen_readouts
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )

#
# Unit tests
#
set(BuilderGdGemReadoutsTest_INC
  BuilderReadouts.h
  ../srs/SRSMappings.h
  )
set(BuilderGdGemReadoutsTest_SRC
  BuilderReadouts.cpp
  BuilderReadoutsTest.cpp
  ../srs/SRSMappings.cpp
  )
create_test_executable(BuilderGdGemReadoutsTest)


set(BuilderGdGemHitsTest_INC
  BuilderHits.h
  #../srs/SRSMappings.h
  )
set(BuilderGdGemHitsTest_SRC
  BuilderHits.cpp
  BuilderHitsTest.cpp
  #../srs/SRSMappings.cpp
  )
create_test_executable(BuilderGdGemHitsTest)
