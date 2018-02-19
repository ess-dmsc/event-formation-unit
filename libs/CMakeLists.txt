include_directories(..)

set(library_SRC
  source/Socket.cpp
  source/Timer.cpp
  source/Thread.cpp
  source/TSCTimer.cpp
  )

set(library_INC
  include/Socket.h
  include/Timer.h
  include/Thread.h
  include/Counter.h
  include/TSCTimer.h
  include/Counter.h
  include/StatCounter.h
  )

add_definitions(-std=c++11)

add_library(eventlib
  ${library_SRC}
  ${library_INC}
  )

enable_coverage(eventlib)
