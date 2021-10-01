#=============================================================================
# NMX Readout generator (h5)
#=============================================================================

set(nmxgen_readouts_SRC
  ${ESS_SOURCE_DIR}/generators/udpgen_hits/generator.cpp
  ReaderReadouts.cpp
  )
set(nmxgen_readouts_INC
  ReaderReadouts.h
  )
create_executable(nmxgen_readouts)
target_compile_definitions(nmxgen_readouts PUBLIC GENERATOR_GDGEM_READOUTS)


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
