# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# bifrost detector module and base test
#=============================================================================


set(bifrost_SRC main.cpp)
set(bifrost_LIB efu_reduction efu_essreadout caen_common)
create_executable(bifrost)

add_dependencies(bifrost caen_common efu_reduction efu_essreadout )
