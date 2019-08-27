#=============================================================================
# Classes for Multi-grid Mesytec parser
#=============================================================================

set(mg_mesytec_obj_SRC
  Readout.cpp
  BuilderReadouts.cpp
  Vmmr16Parser.cpp
  Sis3153Parser.cpp
  BuilderMesytec.cpp
  )

set(mg_mesytec_obj_INC
  Readout.h
  BuilderReadouts.h
  Vmmr16Parser.h
  Sis3153Parser.h
  BuilderMesytec.h
  )

add_library(MgMesytec STATIC
  ${mg_mesytec_obj_SRC}
  ${mg_mesytec_obj_INC}
  )

enable_coverage_flags(MgMesytec)
