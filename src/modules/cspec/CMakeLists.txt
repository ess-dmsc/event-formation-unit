# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# bifrost detector module and base test
#=============================================================================

add_subdirectory(test)


set(cspec_SRC main.cpp)
set(cspec_LIB efu_reduction caen_common efu_essreadout)
create_executable(cspec)
