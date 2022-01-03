# Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Hit generator (h5)
#=============================================================================

add_subdirectory(udpgenpcap)

# \todo ReaderHits needs unit tests?

set(udpgen_hits_SRC
    udpgen_hits/generator.cpp
    udpgen_hits/ReaderHits.cpp
    )
set(udpgen_hits_INC
    udpgen_hits/ReaderHits.h
    )
create_executable(udpgen_hits)
target_compile_definitions(udpgen_hits PUBLIC GENERATOR_UDP_HITS)


# Stream event pixels from hdf5 files
set(hdf5events_SRC hdf5replay/hdf5events.cpp)
create_executable(hdf5events)
