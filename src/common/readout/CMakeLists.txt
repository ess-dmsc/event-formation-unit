
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
# To be used as a dependency in the module's CMakeLists.txt
# aimed to be built with UNIT_TEST macro defined
add_library(EssReadoutLib-unit_test OBJECT
  ${essreadout_obj_SRC}
  ${essreadout_obj_INC}
  )

target_compile_definitions(EssReadoutLib-unit_test PUBLIC -DUNIT_TEST)

enable_coverage_flags(EssReadoutLib)
