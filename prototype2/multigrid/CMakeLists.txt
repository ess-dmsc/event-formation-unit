
# Where does this really need to go ???
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/calib_data
  DESTINATION ${CMAKE_BINARY_DIR}/prototype2)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

# Two-thread implementation of Multi Grid demonstrator detector
# For the old readout system
set(mgcncs2_SRC
  mgcncs2.cpp
  mgcncs/DataParser.cpp
  mgcncs/ChanConv.cpp
  mgcncs/CalibrationFile.cpp
  )
set(mgcncs2_INC
  mgcncs/DataParser.h
  mgcncs/ChanConv.h
  mgcncs/MultigridGeometry.h
  mgcncs/CalibrationFile.h
  )
create_module(mgcncs2)

# Two-thread implementation of Multi Grid demonstrator detector
# for Mesytec Readout
set(mgmesytec_SRC
  ../common/Hists.cpp
  ../common/HistSerializer.cpp
  ../common/ReadoutSerializer.cpp
  mgmesytec.cpp
  mgmesytec/DataParser.cpp
  )
set(mgmesytec_INC
  ../common/Hists.h
  ../common/HistSerializer.h
  ../common/ReadoutSerializer.h
  mgmesytec/DataParser.h
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
set(mgcncsgenfile_INC generators/DGArgs.h )
set(mgcncsgenfile_SRC
  generators/mgcncsgenfile.cpp
  generators/DGArgs.cpp
  )
create_executable(mgcncsgenfile)

# Data generator for raw cspec data
set(mgcncsgenjson_INC generators/JsonArgs.h )
set(mgcncsgenjson_SRC
  generators/mgcncsgenjson.cpp
  generators/JsonArgs.cpp
  ../../dataformats/multigrid/src/RunSpecParse.cpp
  )
create_executable(mgcncsgenjson)

#=============================================================================
# Tests
#=============================================================================

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


set(MGMesytecDataTest_SRC
  ../common/Hists.cpp
  ../common/ReadoutSerializer.cpp
  ../common/Producer.cpp
  mgmesytec/DataParserTest.cpp
  mgmesytec/DataParser.cpp
  )
set(MGMesytecDataTest_INC
  ../common/Hists.h
  ../common/ReadoutSerializer.h
  ../common/Producer.h
  mgmesytec/DataParser.h
  mgmesytec/TestData.h
  )
create_test_executable(MGMesytecDataTest)

set(MG24DetectorTest_SRC
  mgmesytec/MG24DetectorTest.cpp)
set(MG24DetectorTest_INC
  mgmesytec/MG24Detector.h)
create_test_executable(MG24DetectorTest)

#set(CSPECEventTest_SRC cspec/CSPECEventTest.cpp)
#set(CSPECEventTest_INC cspec/CSPECEvent.h)
#create_test_executable(CSPECEventTest)


# This was moved out from the IS_GNUCXX because it is needed
# in efu/CMakeLists
add_library(calibfile OBJECT mgcncs/CalibrationFile.cpp)


if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
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
