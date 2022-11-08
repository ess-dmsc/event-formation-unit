include_directories(.)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================
set(perfgen_INC
  PerfGenBase.h
  ${ESS_SOURCE_DIR}/common/TestImageUdder.h
)
set(perfgen_SRC
  PerfGenBase.cpp
  main.cpp
)
create_executable(perfgen)
target_compile_definitions(perfgen PUBLIC EFU_PERFGEN)
