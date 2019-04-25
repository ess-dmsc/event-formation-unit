set(library_SRC
  source/Socket.cpp
  source/Timer.cpp
  source/TSCTimer.cpp
  )

set(library_INC
  include/Socket.h
  include/Timer.h
  include/TSCTimer.h
  )
  
include_directories(../)

add_library(eventlib STATIC
  ${library_SRC}
  ${library_INC}
  )

set_target_properties(eventlib PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_subdirectory(test)

enable_coverage(eventlib)
