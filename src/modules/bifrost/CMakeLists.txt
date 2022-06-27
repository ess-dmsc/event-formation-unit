
add_subdirectory(generators)

include_directories(.)

#=============================================================================
# bifrost detector module and base test
#=============================================================================
set(bifrost_common_inc
  Counters.h
  BifrostBase.h
  BifrostInstrument.h
  readout/DataParser.h
  geometry/Config.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.h
  )
set(bifrost_common_src
  BifrostBase.cpp
  BifrostInstrument.cpp
  readout/DataParser.cpp
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.cpp
  ${ESS_SOURCE_DIR}/common/debug/Hexdump.cpp
  )
set(bifrost_INC ${bifrost_common_inc})
set(bifrost_SRC Bifrost.cpp ${bifrost_common_src})
create_module(bifrost)
