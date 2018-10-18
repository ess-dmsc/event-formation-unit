#=============================================================================
# APV2VMM Generator (h5)
#=============================================================================

set(nmxgen_apv_SRC
    ../nmx/Hit.cpp
    nmxgen_apv.cpp
    NMXArgs.cpp
    ReaderAPV.cpp
    )
set(nmxgen_apv_INC
    ../nmx/Hit.h
    NMXArgs.h
    ReaderAPV.h
    )
create_executable(nmxgen_apv)

set(BuilderAPVTest_SRC
    ../nmx/Event.cpp
    ../nmx/Cluster.cpp
    ../nmx/Hists.cpp
    ../nmx/AbstractBuilder.cpp
    BuilderAPVTest.cpp
    BuilderAPV.cpp
    )
set(BuilderAPVTest_INC
    ../nmx/Cluster.h
    ../nmx/Event.h
    ../nmx/Hit.h
    ../nmx/Hists.h
    BuilderAPV.h
    )
#create_test_executable(BuilderAPVTest)

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
