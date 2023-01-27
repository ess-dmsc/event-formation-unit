# Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# loki detector module and base test
#=============================================================================

add_subdirectory(generators)
add_subdirectory(test)

set(loki_SRC main.cpp)
set(loki_LIB efu_reduction caen_common efu_essreadout)
create_executable(loki)
