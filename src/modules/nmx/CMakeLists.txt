# Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

#=============================================================================
# nmx detector module and base test
#=============================================================================
set(nmx_common_inc
  NMXBase.h
  NMXInstrument.h
  geometry/Config.h
  geometry/NMXGeometry.h
  geometry/Geometry.h
  )
set(nmx_common_src
  NMXBase.cpp
  NMXInstrument.cpp
  geometry/Config.cpp
  geometry/NMXGeometry.cpp
  )

set(nmx_INC ${nmx_common_inc})
set(nmx_SRC
  ${nmx_common_src}
  main.cpp
  )
set(nmx_LIB efu_reduction efu_essreadout)
create_executable(nmx)

set(NMXBaseTest_INC
  ${nmx_common_inc}
  ${ESS_COMMON_DIR}/testutils/SaveBuffer.h
  ${ESS_COMMON_DIR}/testutils/TestUDPServer.h
)
set(NMXBaseTest_SRC
  test/NMXBaseTest.cpp
  ${ESS_COMMON_DIR}/testutils/SaveBuffer.cpp
  ${nmx_common_src}
  ${ESS_COMMON_DIR}/testutils/TestUDPServer.cpp
)
create_test_executable(NMXBaseTest)


# NMX InstrumentTest Module integration test

set(NMXInstrumentTest_INC
  ${nmx_common_inc}
  ${ESS_COMMON_DIR}/testutils/SaveBuffer.h
  )
set(NMXInstrumentTest_SRC
  ${nmx_common_src}
  test/NMXInstrumentTest.cpp
  ${ESS_COMMON_DIR}/testutils/SaveBuffer.cpp
)
create_test_executable(NMXInstrumentTest)
