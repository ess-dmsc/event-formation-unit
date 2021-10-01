#=============================================================================
# Raw evenlet generator (h5)
#=============================================================================

set(mggen_readouts_SRC
  ${ESS_SOURCE_DIR}/generators/udpgen_hits/generator.cpp
  ../mesytec/Readout.cpp
  ReaderReadouts.cpp
  )
set(mggen_readouts_INC
  ../mesytec/Readout.h
  ReaderReadouts.h
  )

create_executable(mggen_readouts)
target_compile_definitions(mggen_readouts PUBLIC GENERATOR_MULTIGRID_READOUTS)
