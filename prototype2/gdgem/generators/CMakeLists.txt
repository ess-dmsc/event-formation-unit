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
