# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# loki detector module and base test
#=============================================================================


set(loki_SRC main.cpp)
set(loki_LIB efu_reduction caen_common efu_essreadout)
create_executable(loki)
