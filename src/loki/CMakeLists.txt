include_directories(.)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================
set(loki_common_inc
  LokiBase.h
  )
set(loki_common_src
  LokiBase.cpp
  )

set(loki_INC
  ${loki_common_inc}
)
set(loki_SRC
  Loki.cpp
  ${loki_common_src}
)
create_module(loki)

# Pipeline test
set(LokiBaseTest_INC
  ${loki_common_inc}
  ../adc_readout/test/TestUDPServer.h
)
set(LokiBaseTest_SRC
  LokiBaseTest.cpp
  ${loki_common_src}
  ../adc_readout/test/TestUDPServer.cpp
)
create_test_executable(LokiBaseTest)

add_subdirectory(readout)
add_subdirectory(test)
