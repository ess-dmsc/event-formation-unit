#=============================================================================
# Classes for Multi-grid geometry
#=============================================================================

set(mg_geometry_obj_SRC
  Filter.cpp
  ChannelMappings.cpp
  MGSeqMappings.cpp
#  MG24Mappings.cpp
  DetectorMappings.cpp
  )

set(mg_geometry_obj_INC
  Filter.h
  ChannelMappings.h
  MGSeqMappings.h
  MG24Mappings.h
  DetectorMappings.h
  )

add_library(MgGeometry STATIC
  ${mg_geometry_obj_SRC}
  ${mg_geometry_obj_INC}
  )

enable_coverage_flags(MgGeometry)
