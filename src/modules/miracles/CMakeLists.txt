# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# miracles detector module and base test
#=============================================================================

add_subdirectory(test)


set(miracles_SRC main.cpp)
set(miracles_LIB efu_reduction caen_common efu_essreadout)
create_executable(miracles)
