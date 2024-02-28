# Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

get_filename_component(TTLMON_CONFIG "${ESS_MODULE_DIR}/cbm/configs/lokimon.json" ABSOLUTE)

#=============================================================================
# ttlmonitor detector module and base test
#=============================================================================
set(ttlmonitor_common_inc
  TTLMonitorBase.h
  TTLMonitorInstrument.h
  geometry/Config.h
  geometry/Parser.h
  )


set(ttlmonitor_common_src
  TTLMonitorBase.cpp
  TTLMonitorInstrument.cpp
  geometry/Config.cpp
  geometry/Parser.cpp
  )

#
set(ttlmonitor_INC ${ttlmonitor_common_inc})
set(ttlmonitor_SRC
  ${ttlmonitor_common_src}
  main.cpp
  )
set(ttlmonitor_LIB efu_essreadout)
create_executable(ttlmonitor)


#
set(TTLMonitorBaseTest_INC
  ${ttlmonitor_common_inc}
)
set(TTLMonitorBaseTest_SRC
  test/TTLMonitorBaseTest.cpp
  ${ttlmonitor_common_src}
)
create_test_executable(TTLMonitorBaseTest)
target_compile_definitions(TTLMonitorBaseTest PRIVATE TTLMON_CONFIG="${TTLMON_CONFIG}")

#
# TTLMonitoriInstrumentTest Module integration test
#
set(TTLMonitorInstrumentTest_INC
  ${ttlmonitor_common_inc}
  )
set(TTLMonitorInstrumentTest_SRC
  ${ttlmonitor_common_src}
  test/TTLMonitorInstrumentTest.cpp
)
create_test_executable(TTLMonitorInstrumentTest)
target_compile_definitions(TTLMonitorInstrumentTest PRIVATE TTLMON_CONFIG="${TTLMON_CONFIG}")
