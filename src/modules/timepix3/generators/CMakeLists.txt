# Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# tpx_generator for timepix3 efu
#=============================================================================

set(tpx_generator_PY
  tpx_generator.py
)

install_python_executable(tpx_generator)
set_target_properties(tpx_generator PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/generators")