
#=============================================================================
# Add ess timin modules
#=============================================================================

include_directories(.)

set(esstime_obj_SRC
  ESSTime.cpp
  Timer.cpp
  TimeString.cpp
  TSCTimer.cpp
  CheckTimer.cpp
)

set(esstime_obj_INC
ESSTime.h
Timer.h
TimeString.h
TSCTimer.h
CheckTimer.h
)

add_library(EssTimingLib OBJECT
  ${esstime_obj_SRC}
  ${esstime_obj_INC}
  )

enable_coverage_flags(EssTimingLib)
