#=============================================================================
# Raw evenlet generator (h5)
#=============================================================================

set(nmxgen_hits_SRC
  ../../common/clustering/Hit.cpp
    nmxgen_hits.cpp
    ReaderHits.cpp
    NMXArgs.cpp
    )
set(nmxgen_hits_INC
  ../../common/clustering/Hit.h
    ReaderHits.h
    NMXArgs.h
    )
create_executable(nmxgen_hits)
