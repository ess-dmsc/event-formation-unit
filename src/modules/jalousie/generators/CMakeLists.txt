#=============================================================================
# PCAP generator
#=============================================================================
include_directories(.)
include_directories(..)

# TODO REMOVE?
set(udpgen_jalousie_SRC
  ${ESS_MODULE_DIR}/generators/generator.cpp
  CdtFile.cpp
  ../Readout.cpp
  )
set(udpgen_jalousie_INC
  CdtFile.h
  ../Readout.h
  )
create_executable(udpgen_jalousie)
target_compile_definitions(udpgen_jalousie PUBLIC GENERATOR_JALOUSIE_CDT)
