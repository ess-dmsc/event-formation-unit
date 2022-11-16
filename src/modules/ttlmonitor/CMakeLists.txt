# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

#=============================================================================
# ttlmonitor detector module and base test
#=============================================================================
set(ttlmonitor_common_inc
  TTLMonitorBase.h
  TTLMonitorInstrument.h
  geometry/Config.h
  )


set(ttlmonitor_common_src
  TTLMonitorBase.cpp
  TTLMonitorInstrument.cpp
  geometry/Config.cpp
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
  ${ESS_COMMON_DIR}/testutils/SaveBuffer.h
  ${ESS_COMMON_DIR}/testutils/TestUDPServer.h
)
set(TTLMonitorBaseTest_SRC
  test/TTLMonitorBaseTest.cpp
  ${ESS_COMMON_DIR}/testutils/SaveBuffer.cpp
  ${ttlmonitor_common_src}
  ${ESS_COMMON_DIR}/testutils/TestUDPServer.cpp
)
create_test_executable(TTLMonitorBaseTest)


#
# TTLMonitoriInstrumentTest Module integration test
#
set(TTLMonitorInstrumentTest_INC
  ${ttlmonitor_common_inc}
  ${ESS_COMMON_DIR}/testutils/SaveBuffer.h
  )
set(TTLMonitorInstrumentTest_SRC
  ${ttlmonitor_common_src}
  test/TTLMonitorInstrumentTest.cpp
  ${ESS_COMMON_DIR}/testutils/SaveBuffer.cpp
)
create_test_executable(TTLMonitorInstrumentTest)
