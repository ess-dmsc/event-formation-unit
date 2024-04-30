# Copyright (C) 2021-2024 European Spallation Source, ERIC. See LICENSE file
#=============================================================================

add_subdirectory(functiongenerators)
add_subdirectory(udpgenpcap)

# Candidate for removal
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
