cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

include_directories(.. ../.. )
#
# Detector Pipeline Plugins
#

# Multi Grid demonstrator detector - obsolete
#set(mgcncs_SRC mgcncs.cpp mgcncs/Data.cpp mgcncs/ChanConv.cpp)
#set(mgcncs_INC mgcncs/Data.h mgcncs/ChanConv.h mgcncs/MultigridGeometry.h)
#create_module(mgcncs "")


# Two-thread implementation of Multi Grid demonstrator detector
# For the old readout system
set(mgcncs2_SRC
  mgcncs2.cpp
  mgcncs/Data.cpp
  mgcncs/ChanConv.cpp
  mgcncs/CalibrationFile.cpp)
set(mgcncs2_INC
  mgcncs/Data.h
  mgcncs/ChanConv.h
  mgcncs/MultigridGeometry.h
  mgcncs/CalibrationFile.h)
create_module(mgcncs2 "")

# Two-thread implementation of Multi Grid demonstrator detector
# for Mesytec Readout
set(mgmesytec_SRC
  ../gdgem/nmx/Hists.cpp
  ../gdgem/nmx/HistSerializer.cpp
  ../common/ReadoutSerializer.cpp
  mgmesytec.cpp
  mgmesytec/Data.cpp)
set(mgmesytec_INC
  ../gdgem/nmx/Hists.h
  ../gdgem/nmx/HistSerializer.h
  ../common/ReadoutSerializer.h
  mgmesytec/Data.h)
create_module(mgmesytec "")


#
# Data Generators
#

# Generates dummy 'CSPEC' data matching the Multi Grid demonstrator
set(mgcncsgen_SRC
  mgcncsgen/mgcncsgen.cpp
  mgcncsgen/DGArgs.cpp
  mgcncs/Data.cpp)
set(mgcncsgen_INC
  mgcncsgen/DGArgs.h
  mgcncs/Data.h)
create_executable(mgcncsgen "")

#
set(mgcncsgenfile_SRC
  mgcncsgen/mgcncsgenfile.cpp
  mgcncsgen/DGArgs.cpp)
set(mgcncsgenfile_INC
  mgcncsgen/DGArgs.h)
create_executable(mgcncsgenfile "")

# Data generator for raw cspec data
set(mgcncsgenjson_SRC
  mgcncsgen/mgcncsgenjson.cpp
  mgcncsgen/JsonArgs.cpp
  ../../dataformats/multigrid/src/RunSpecParse.cpp
  ../../dataformats/multigrid/src/jsoncpp.cpp)
set(mgcncsgenjson_INC mgcncsgen/JsonArgs.h)
create_executable(mgcncsgenjson "")

enable_testing()
#
# Multi Grid Tests
#
set(MGCNCSChanConvTest_SRC
  mgcncs/ChanConvTest.cpp
  mgcncs/ChanConv.cpp)
create_test_executable(MGCNCSChanConvTest "")

set(MGCNCSDataTest_SRC
  mgcncs/DataTest.cpp
  mgcncs/Data.cpp
  mgcncs/ChanConv.cpp
  )
set(MGCNCSDataTest_INC
  mgcncs/Data.h
  mgcncs/ChanConv.h
  mgcncs/TestData.h
  mgcncs/MultigridGeometry.h)
create_test_executable(MGCNCSDataTest "")


set(MGMesytecDataTest_SRC
  ../gdgem/nmx/Hists.cpp
  ../common/ReadoutSerializer.cpp
  ../common/Producer.cpp
  mgmesytec/DataTest.cpp
  mgmesytec/Data.cpp)
set(MGMesytecDataTest_INC
  ../gdgem/nmx/Hists.h
  ../common/ReadoutSerializer.h
  ../common/Producer.h
  mgmesytec/Data.h
  mgmesytec/TestData.h)
create_test_executable(MGMesytecDataTest "")

set(MGSEQDetectorTest_SRC
mgmesytec/MGSEQDetectorTest.cpp)
set(MGSEQDetectorTest_INC
mgmesytec/MGSEQDetector.h)
create_test_executable(MGSEQDetectorTest "")

#set(CSPECEventTest_SRC cspec/CSPECEventTest.cpp)
#set(CSPECEventTest_INC cspec/CSPECEvent.h)
#create_test_executable(CSPECEventTest "")

# Is this ever used?
set(PYTHON_EXECUTABLE /usr/bin/python)


# This possibly only gets used by the thing below
# Is this really the best way to do this?
add_library(calibfile OBJECT mgcncs/CalibrationFile.cpp)
enable_coverage_flags(calibfile)


if (${CMAKE_COMPILER_IS_GNUCXX})
  set(CalibrationFileTest_LIB_SRC
    mgcncs/CalibrationFileTest.cpp
    ../test/SyscallWrappers.cpp)
  add_library(CalibrationFileTest_LIB
    STATIC ${CalibrationFileTest_LIB_SRC})
  set_target_properties(
    CalibrationFileTest_LIB PROPERTIES
    COMPILE_FLAGS "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")

  set(CalibrationFileTest_SRC $<TARGET_OBJECTS:calibfile>)
  set(CalibrationFileTest_INC mgcncs/CalibrationFile.h)
  set(CalibrationFileTest_LINK CalibrationFileTest_LIB ${GTEST_BOTH_LIBRARIES})
  create_test_executable(CalibrationFileTest "${CalibrationFileTest_LINK}")
  add_compile_flags(CalibrationFileTest
    "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
  add_linker_flags(CalibrationFileTest
    "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
endif ()
