#=============================================================================
# NMX Readout generator (h5)
#=============================================================================

set(nmxgen_readouts_SRC
  ../../generators/generator.cpp
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
# TODO BuilderReadoutsTest is maybe too generic, how about similar for
# Multi-Blade ?
set(BuilderReadoutsTest_INC
  BuilderReadouts.h
  ../srs/SRSMappings.h
  )
set(BuilderReadoutsTest_SRC
  BuilderReadouts.cpp
  BuilderReadoutsTest.cpp
  ../srs/SRSMappings.cpp
  )
create_test_executable(BuilderReadoutsTest)
