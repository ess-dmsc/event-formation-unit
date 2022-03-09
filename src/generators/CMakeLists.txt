# Copyright (C) 2021-2022 European Spallation Source, ERIC. See LICENSE file
#=============================================================================
# Hit generator (h5)
# Sends raw Hits as udp packets - this is probably obsolete
#=============================================================================

add_subdirectory(udpgenpcap)

# \todo ReaderHits - is this obsolete?

set(obsolete_udp_hits_from_h5_SRC
    udpgen_hits/generator.cpp
    udpgen_hits/ReaderHits.cpp
    )
set(obsolete_udp_hits_from_h5_INC
    udpgen_hits/ReaderHits.h
    )
create_executable(obsolete_udp_hits_from_h5)
target_compile_definitions(obsolete_udp_hits_from_h5
  PUBLIC GENERATOR_UDP_HITS
  )
set_target_properties(obsolete_udp_hits_from_h5
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )


#=============================================================================
# Stream ev42 {pixels} from hdf5 files
#=============================================================================

set(kafka_ev42_from_h5_SRC
  hdf5replay/hdf5events.cpp
  )
create_executable(kafka_ev42_from_h5)
set_target_properties(kafka_ev42_from_h5
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators"
  )
