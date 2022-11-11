# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# loki detector module and base test
#=============================================================================


set(loki_SRC main.cpp)
set(loki_LIB efu_reduction efu_essreadout caen_common)
create_executable(loki)

target_link_libraries(loki caen_common efu_reduction efu_essreadout)
