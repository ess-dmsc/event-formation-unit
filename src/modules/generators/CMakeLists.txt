# Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file

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
target_compile_definitions(udpgen_hits PUBLIC GENERATOR_UDP_HITS)


# Stream event pixels from hdf5 files
set(hdf5events_SRC hdf5events.cpp)
create_executable(hdf5events)
