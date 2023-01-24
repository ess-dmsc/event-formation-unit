include_directories(.)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================
set(perfgen_INC
  PerfGenBase.h
  ${ESS_COMMON_DIR}/TestImageUdder.h
)
set(perfgen_SRC
  PerfGenBase.cpp
  main.cpp
)
create_executable(perfgen)
