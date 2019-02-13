include_directories(. ..)

set(udptx_SRC
  udptx.cpp
  Args.cpp
  )
set(udptx_INC
  Args.h
  )
add_executable(udptx
  ${udptx_SRC}
  ${udptx_INC}
  )
target_link_libraries(udptx eventlib efu_common)
set_target_properties(udptx PROPERTIES
  COMPILE_FLAGS "-fno-strict-aliasing")

set(udprx_SRC
  udprx.cpp
  Args.cpp
  )
set(udprx_INC
  Args.h
  )
add_executable(udprx
  ${udprx_SRC}
  ${udprx_INC}
  )
target_link_libraries(udprx eventlib efu_common)

install(TARGETS
  udptx
  udprx
  DESTINATION bin)
