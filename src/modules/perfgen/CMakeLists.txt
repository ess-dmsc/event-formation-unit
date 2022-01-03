include_directories(.)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================
set(perfgen_INC
  PerfGenBase.h
  ${ESS_SOURCE_DIR}/common/TestImageUdder.h
)
set(perfgen_SRC
  PerfGen.cpp
  PerfGenBase.cpp
)
create_module(perfgen)
