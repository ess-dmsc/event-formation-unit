#=============================================================================
# Raw evenlet generator (h5)
#=============================================================================

set(nmxgen_hits_SRC
    ../nmx/Hit.cpp
    nmxgen_hits.cpp
    ReaderHits.cpp
    NMXArgs.cpp
    )
set(nmxgen_hits_INC
    ../nmx/Hit.h
    ReaderHits.h
    NMXArgs.h
    )
create_executable(nmxgen_hits)
