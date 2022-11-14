# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# bifrost detector module and base test
#=============================================================================


set(bifrost_SRC main.cpp)
set(bifrost_LIB efu_reduction caen_common efu_essreadout)
create_executable(bifrost)
