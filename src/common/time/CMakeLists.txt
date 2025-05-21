
#=============================================================================
# Add ess timing modules
#=============================================================================

include_directories(.)

set(esstime_obj_SRC
  ESSTime.cpp
  TimeString.cpp
  Timer.cpp
)

set(esstime_obj_INC
  ESSTime.h
  TimeString.h
  Timer.h
)

add_library(EssTimingLib OBJECT
  ${esstime_obj_SRC}
  ${esstime_obj_INC}
  )

target_link_libraries(EssTimingLib
  PRIVATE
    ${EFU_COMMON_LIBS}
)

add_library(EssTimingLib-unit_test OBJECT
  ${esstime_obj_SRC}
  ${esstime_obj_INC}
  )

# Set pre-compiler flag UNIT_TEST for the library
target_compile_definitions(EssTimingLib-unit_test PUBLIC -DUNIT_TEST)

enable_coverage_flags(EssTimingLib)
