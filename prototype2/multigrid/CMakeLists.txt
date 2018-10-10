
# Where does this really need to go ???
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/calib_data
  DESTINATION ${CMAKE_BINARY_DIR}/prototype2)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

# Two-thread implementation of Multi Grid demonstrator detector
# for Mesytec Readout
set(mgmesytec_common_src
  MGMesytecBase.cpp
  ../common/Hists.cpp
  ../common/HistSerializer.cpp
  ../common/ReadoutSerializer.cpp
  MgConfig.cpp
  mgmesytec/EfuMaximum.cpp
  mgmesytec/EfuCenterMass.cpp
  mgmesytec/EfuPrioritized.cpp
  mgmesytec/Vmmr16Parser.cpp
  mgmesytec/Sis3153Parser.cpp
  mgmesytec/Hit.cpp
  mgmesytec/HitFile.cpp
  )
set(mgmesytec_common_inc
  MGMesytecBase.h
  ../common/Hists.h
  ../common/HistSerializer.h
  ../common/ReadoutSerializer.h
  MgConfig.h
  mgmesytec/Efu.h
  mgmesytec/EfuMaximum.h
  mgmesytec/EfuCenterMass.h
  mgmesytec/EfuPrioritized.h
  mgmesytec/Vmmr16Parser.h
  mgmesytec/Sis3153Parser.h
  mgmesytec/Hit.h
  mgmesytec/HitFile.h
  mgmesytec/SequoiaGeometry.h
  mgmesytec/BusGeometry.h
  )

set(mgmesytec_INC
  ${mgmesytec_common_inc}
)
set(mgmesytec_SRC
  MGMesytec.cpp
  ${mgmesytec_common_src}
)
create_module(mgmesytec)


#=============================================================================
# Data generators
#=============================================================================

# Generates dummy 'CSPEC' data matching the Multi Grid demonstrator
set(mgcncsgen_SRC
  generators/mgcncsgen.cpp
  generators/DGArgs.cpp
  mgcncs/DataParser.cpp
  )
set(mgcncsgen_INC
  generators/DGArgs.h
  mgcncs/DataParser.h
  )
create_executable(mgcncsgen)

#
set(mgcncsgenfile_INC generators/DGArgs.h)
set(mgcncsgenfile_SRC
  generators/mgcncsgenfile.cpp
  generators/DGArgs.cpp
  )
create_executable(mgcncsgenfile)

# Data generator for raw cspec data
set(mgcncsgenjson_INC generators/JsonArgs.h)
set(mgcncsgenjson_SRC
  generators/mgcncsgenjson.cpp
  generators/JsonArgs.cpp
  ../../dataformats/multigrid/src/RunSpecParse.cpp
  )
create_executable(mgcncsgenjson)

#=============================================================================
# Tests
#=============================================================================

set(MGMesytecBaseTest_INC
  ${mgmesytec_common_inc}
  ../../libs/include/Socket.h
  ../../libs/include/TSCTimer.h
  ../adc_readout/test/TestUDPServer.h
)
set(MGMesytecBaseTest_SRC
  ${mgmesytec_common_src}
  MGMesytecBaseTest.cpp
  ../../libs/source/Socket.cpp
  ../../libs/source/TSCTimer.cpp
  ../adc_readout/test/TestUDPServer.cpp
)
create_test_executable(MGMesytecBaseTest)

set(MgConfigTest_SRC
  MgConfig.cpp
  MgConfigTest.cpp
  ../../prototype2/common/DataSave.cpp
)
set(MgConfigTest_INC
  MgConfig.h
  ../../prototype2/common/DataSave.h
)
create_test_executable(MgConfigTest)


set(MGCNCSChanConvTest_SRC
  mgcncs/ChanConvTest.cpp
  mgcncs/ChanConv.cpp
  )
create_test_executable(MGCNCSChanConvTest)

set(MGCNCSDataTest_SRC
  mgcncs/DataParserTest.cpp
  mgcncs/DataParser.cpp
  mgcncs/ChanConv.cpp
  )
set(MGCNCSDataTest_INC
  mgcncs/DataParser.h
  mgcncs/ChanConv.h
  mgcncs/TestData.h
  mgcncs/MultigridGeometry.h
  )
create_test_executable(MGCNCSDataTest)

set(MGHitTest_SRC
  mgmesytec/HitTest.cpp
  mgmesytec/Hit.cpp
  )
set(MGHitTest_INC
  mgmesytec/Hit.h
  )
create_test_executable(MGHitTest)


set(MGHitFileTest_SRC
  mgmesytec/HitFileTest.cpp
  mgmesytec/HitFile.cpp
  mgmesytec/Hit.cpp
  )
set(MGHitFileTest_INC
  mgmesytec/HitFile.h
  mgmesytec/Hit.h
  )
create_test_executable(MGHitFileTest SKIP_MEMGRIND)

set(Sis3153ParserTest_SRC
  mgmesytec/Sis3153ParserTest.cpp
  mgmesytec/Sis3153Parser.cpp
  )
set(Sis3153ParserTest_INC
  mgmesytec/Sis3153Parser.h
  mgmesytec/TestData.h
  ../common/Buffer.h
  )
create_test_executable(Sis3153ParserTest)

set(Vmmr16ParserTest_SRC
  mgmesytec/Vmmr16ParserTest.cpp
  mgmesytec/Vmmr16Parser.cpp
  mgmesytec/Sis3153Parser.cpp
  mgmesytec/Hit.cpp
  )
set(Vmmr16ParserTest_INC
  ../common/Buffer.h
  mgmesytec/Sis3153Parser.h
  mgmesytec/Vmmr16Parser.h
  mgmesytec/Hit.h
  mgmesytec/TestData.h
  )
create_test_executable(Vmmr16ParserTest)

set(MG24GeometryTest_SRC
  mgmesytec/MG24GeometryTest.cpp)
set(MG24GeometryTest_INC
  mgmesytec/MG24Geometry.h)
create_test_executable(MG24GeometryTest)

set(FilterTest_SRC
  mgmesytec/FilterTest.cpp)
set(FilterTest_INC
  mgmesytec/Filter.h)
create_test_executable(FilterTest)

set(BusGeometryTest_SRC
  mgmesytec/BusGeometryTest.cpp)
set(BusGeometryTest_INC
  mgmesytec/BusGeometry.h)
create_test_executable(BusGeometryTest)

set(SequoiaGeometryTest_SRC
  mgmesytec/SequoiaGeometryTest.cpp)
set(SequoiaGeometryTest_INC
  mgmesytec/SequoiaGeometry.h)
create_test_executable(SequoiaGeometryTest)

set(EfuMaximumTest_SRC
  mgmesytec/EfuMaximumTest.cpp
  mgmesytec/EfuMaximum.cpp
  mgmesytec/Hit.cpp
  ../common/Hists.cpp
  ../common/ReadoutSerializer.cpp)
set(EfuMaximumTest_INC
  mgmesytec/EfuMaximum.h
  mgmesytec/Efu.h
  mgmesytec/SequoiaGeometry.h
  mgmesytec/Hit.h
  ../common/Hists.h
  ../common/ReadoutSerializer.h)
create_test_executable(EfuMaximumTest)

set(EfuCenterMassTest_SRC
  mgmesytec/EfuCenterMassTest.cpp
  mgmesytec/EfuCenterMass.cpp
  mgmesytec/Hit.cpp
  ../common/Hists.cpp
  ../common/ReadoutSerializer.cpp)
set(EfuCenterMassTest_INC
  mgmesytec/EfuCenterMass.h
  mgmesytec/Efu.h
  mgmesytec/SequoiaGeometry.h
  mgmesytec/Hit.h
  ../common/Hists.h
  ../common/ReadoutSerializer.h)
create_test_executable(EfuCenterMassTest)

set(EfuPrioritizedTest_SRC
  mgmesytec/EfuPrioritizedTest.cpp
  mgmesytec/EfuPrioritized.cpp
  mgmesytec/Hit.cpp
  ../common/Hists.cpp
  ../common/ReadoutSerializer.cpp)
set(EfuPrioritizedTest_INC
  mgmesytec/EfuCenterMass.h
  mgmesytec/Efu.h
  mgmesytec/SequoiaGeometry.h
  mgmesytec/Hit.h
  ../common/Hists.h
  ../common/ReadoutSerializer.h)
create_test_executable(EfuPrioritizedTest)

#set(CSPECEventTest_SRC cspec/CSPECEventTest.cpp)
#set(CSPECEventTest_INC cspec/CSPECEvent.h)
#create_test_executable(CSPECEventTest)


# This was moved out from the IS_GNUCXX because it is needed
# in efu/CMakeLists
add_library(calibfile OBJECT mgcncs/CalibrationFile.cpp)


if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
  message(STATUS "*** Skipping CalibrationFileTest - fails with Release compile flags")
elseif(${CMAKE_COMPILER_IS_GNUCXX})
  enable_coverage_flags(calibfile)

  set(CalibrationFileTestLibrary_SRC
    mgcncs/CalibrationFileTest.cpp
    ../test/SyscallWrappers.cpp)
  add_library(CalibrationFileTestLibrary
    STATIC ${CalibrationFileTestLibrary_SRC})
  add_compile_flags(CalibrationFileTestLibrary
    "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
  enable_coverage_flags(CalibrationFileTestLibrary)

  set(CalibrationFileTest_SRC $<TARGET_OBJECTS:calibfile>)
  set(CalibrationFileTest_INC mgcncs/CalibrationFile.h)
  set(CalibrationFileTest_LIB CalibrationFileTestLibrary ${GTEST_BOTH_LIBRARIES})
  create_test_executable(CalibrationFileTest)
  add_compile_flags(CalibrationFileTest
    "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
  add_linker_flags(CalibrationFileTest
    "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
else()
  message(STATUS "*** Skipping CalibrationFileTest - requires gcc/linux")
endif()

set(MultiGridGeometryTest_SRC
  mgcncs/MultigridGeometryTest.cpp)
set(MultiGridGeometryTest_INC
  mgcncs/MultigridGeometry.h
  ../../libs/include/TSCTimer.h)
create_test_executable(MultiGridGeometryTest)
