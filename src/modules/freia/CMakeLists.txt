# Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)


set(freia_SRC main.cpp)
set(freia_LIB efu_reduction vmm_common efu_essreadout)
create_executable(freia)
