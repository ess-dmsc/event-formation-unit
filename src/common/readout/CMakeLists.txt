
#=============================================================================
# Add ess detector modules
#=============================================================================

include_directories(.)

add_subdirectory(ess)
add_subdirectory(vmm3)


set(essreadout_obj_SRC
  ess/Parser.cpp
  vmm3/VMM3Calibration.cpp
  vmm3/VMM3Config.cpp
  vmm3/VMM3Parser.cpp
)

set(essreadout_obj_INC
  ess/ESSTime.h
  ess/Parser.h
  vmm3/Hybrid.h
  vmm3/Readout.h
  vmm3/VMM3Calibration.h
  vmm3/VMM3Config.h
  vmm3/VMM3Parser.h
)

add_library(EssReadoutLib OBJECT
  ${essreadout_obj_SRC}
  ${essreadout_obj_INC}
  )

enable_coverage_flags(EssReadoutLib)
