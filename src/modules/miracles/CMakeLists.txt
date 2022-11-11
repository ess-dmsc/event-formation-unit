# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# miracles detector module and base test
#=============================================================================


set(miracles_SRC main.cpp)
set(miracles_LIB efu_reduction efu_essreadout caen_common)
create_executable(miracles)

add_dependencies(miracles caen_common efu_reduction efu_essreadout)
