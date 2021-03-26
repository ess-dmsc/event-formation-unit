#=============================================================================
# Hit generator (h5)
#=============================================================================

# \todo ReaderHits needs unit tests?

set(udpgen_hits_SRC
    generator.cpp
    ReaderHits.cpp
    )
set(udpgen_hits_INC
    ReaderHits.h
    )
create_executable(udpgen_hits)
target_compile_definitions(udpgen_hits PUBLIC ESSUDPGEN_DREAM_SIM)
