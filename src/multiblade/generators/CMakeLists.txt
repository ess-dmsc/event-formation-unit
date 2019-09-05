#=============================================================================
# Raw readout generator (h5)
#=============================================================================

set(mbgen_hits_SRC
    mbgen_hits.cpp
    ../caen/Readout.cpp
    ReaderHits.cpp
    )
set(mbgen_hits_INC
    ReaderHits.h
    ../caen/Readout.h
    )
create_executable(mbgen_hits)
