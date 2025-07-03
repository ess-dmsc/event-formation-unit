include_directories(.)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================
set(perfgen_INC
  PerfGenBase.h
)
set(perfgen_SRC
  PerfGenBase.cpp
  main.cpp
)

set(perfgen_LIB efu_essreadout)
create_executable(perfgen)
