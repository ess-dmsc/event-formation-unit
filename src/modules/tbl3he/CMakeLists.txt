# Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)

#=============================================================================
# detector for tbl based on 3He tubes
#=============================================================================

set(tbl3he_INC
  geometry/Tbl3HeConfig.h
  geometry/Tbl3HeGeometry.h
)
set(tbl3he_SRC
  geometry/Tbl3HeConfig.cpp
  geometry/Tbl3HeGeometry.cpp
  main.cpp
)
set(tbl3he_LIB efu_reduction caen_common efu_essreadout)
create_executable(tbl3he)
