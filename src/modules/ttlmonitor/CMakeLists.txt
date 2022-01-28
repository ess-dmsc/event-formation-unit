# Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file

#add_subdirectory(test)
include_directories(.)

#=============================================================================
# ttlmonitor detector module and base test
#=============================================================================
set(ttlmonitor_common_inc
  TTLMonitorBase.h
  TTLMonitorInstrument.h
  geometry/Config.h
  ${ESS_SOURCE_DIR}/common/readout/ess/Parser.h
  ${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.h
  )


set(ttlmonitor_common_src
TTLMonitorBase.cpp
TTLMonitorInstrument.cpp
geometry/Config.cpp
${ESS_SOURCE_DIR}/common/readout/ess/Parser.cpp
${ESS_SOURCE_DIR}/common/readout/vmm3/VMM3Parser.cpp
)

#
set(ttlmonitor_INC ${ttlmonitor_common_inc})
set(ttlmonitor_SRC TTLMonitor.cpp ${ttlmonitor_common_src})
create_module(ttlmonitor)


#
#set(TTLMonitorBaseTest_INC
#  ${ttlmonitor_common_inc}
#  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
#  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
#)
#set(TTLMonitorBaseTest_SRC
#  TTLMonitorBaseTest.cpp
#  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
#  ${freia_common_src}
#  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
#)
#create_test_executable(TTLMonitorBaseTest)


#
# TTLMonitoriInstrumentTest Module integration test
#
set(TTLMonitorInstrumentTest_INC
  ${ttlmonitor_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(TTLMonitorInstrumentTest_SRC
  ${ttlmonitor_common_src}
  test/TTLMonitorInstrumentTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
)
create_test_executable(TTLMonitorInstrumentTest)
