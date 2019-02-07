#=============================================================================
# Raw evenlet generator (h5)
#=============================================================================

set(nmxgen_hits_SRC
  ../../common/clustering/Hit.cpp
    nmxgen_hits.cpp
    ReaderHits.cpp
    )
set(nmxgen_hits_INC
  ../../common/clustering/Hit.h
    ReaderHits.h
    )
create_executable(nmxgen_hits)

set(nmxgen_readouts_SRC
  ../../common/clustering/Hit.cpp
  nmxgen_readouts.cpp
  ReaderReadouts.cpp
  )
set(nmxgen_readouts_INC
  ../../common/clustering/Hit.h
  ReaderReadouts.h
  )
create_executable(nmxgen_readouts)
