# Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# detector for tbl based on 3He tubes
#=============================================================================


set(tbl3he_SRC main.cpp)
set(tbl3he_LIB efu_reduction caen_common efu_essreadout)
create_executable(tbl3he)
