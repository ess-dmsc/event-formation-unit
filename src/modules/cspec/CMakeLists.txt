# Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file

add_subdirectory(test)
add_subdirectory(generators)
include_directories(.)

#=============================================================================
# cspec detector module and base test
#=============================================================================
set(cspec_common_inc
  CSPECBase.h
  CSPECInstrument.h
  geometry/Config.h
  geometry/CSPECGeometry.h
  geometry/LETGeometry.h
  geometry/Geometry.h
  )
set(cspec_common_src
  CSPECBase.cpp
  CSPECInstrument.cpp
  geometry/Config.cpp
  geometry/CSPECGeometry.cpp
  geometry/LETGeometry.cpp
  )

set(cspec_INC
  ${cspec_common_inc}
  )
set(cspec_SRC
  main.cpp
  ${cspec_common_src}
  )
set(cspec_LIB
  efu_reduction
  efu_essreadout
  )
create_executable(cspec)

add_dependencies(cspec efu_reduction efu_essreadout)
target_compile_definitions(cspec PUBLIC EFU_CSPEC)


set(CSPECBaseTest_INC
  ${cspec_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
)
set(CSPECBaseTest_SRC
  test/CSPECBaseTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${cspec_common_src}
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
)
create_test_executable(CSPECBaseTest)

#
# CSPEC InstrumentTest Module integration test
#
set(CSPECInstrumentTest_INC
  ${cspec_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(CSPECInstrumentTest_SRC
  ${cspec_common_src}
  test/CSPECInstrumentTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
)
create_test_executable(CSPECInstrumentTest)
