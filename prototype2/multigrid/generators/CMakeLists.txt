#=============================================================================
# Raw evenlet generator (h5)
#=============================================================================

set(mggen_readouts_SRC
  mggen_readouts.cpp
  ../../common/clustering/Hit.cpp
  ../mesytec/Readout.cpp
  ReaderReadouts.cpp
  )
set(mggen_readouts_INC
  ../../common/clustering/Hit.h
  ../mesytec/Readout.h
  ReaderReadouts.h
  )
create_executable(mggen_readouts)
