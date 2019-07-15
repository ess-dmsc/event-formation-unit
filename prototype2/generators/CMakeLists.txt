#=============================================================================
# Hit generator (h5)
#=============================================================================

set(udpgen_hits_SRC
    generator.cpp
    ReaderHits.cpp
    )
set(udpgen_hits_INC
    ReaderHits.h
    )
create_executable(udpgen_hits)
target_compile_definitions(udpgen_hits PUBLIC GENERATOR_UDP_HITS)
