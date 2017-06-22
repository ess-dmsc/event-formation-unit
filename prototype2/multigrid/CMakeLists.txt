cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

include_directories(..)
#
# Detector Pipeline Plugins
#

# Multi Grid demonstrator detector
set(mgcncs_SRC mgcncs.cpp mgcncs/Data.cpp mgcncs/ChanConv.cpp)
set(mgcncs_INC mgcncs/Data.h mgcncs/ChanConv.h mgcncs/Geometry.h)
create_module(mgcncs "")


# Two-thread implementation of Multi Grid demonstrator detector
set(mgcncs2_SRC mgcncs2.cpp mgcncs/Data.cpp mgcncs/ChanConv.cpp)
set(mgcncs2_INC mgcncs/Data.h mgcncs/ChanConv.h mgcncs/Geometry.h)
create_module(mgcncs2 "")



#
# Data Generators
#

# Generates dummy 'CSPEC' data matching the Multi Grid demonstrator
set(mgcncsgen_SRC mgcncsgen/mgcncsgen.cpp mgcncsgen/DGArgs.cpp mgcncs/Data.cpp)
set(mgcncsgen_INC mgcncsgen/DGArgs.h mgcncs/Data.h)
create_executable(mgcncsgen  "")

#
set(mgcncsgenfile_SRC mgcncsgen/mgcncsgenfile.cpp mgcncsgen/DGArgs.cpp)
set(mgcncsgenfile_INC mgcncsgen/DGArgs.h)
create_executable(mgcncsgenfile "")

# Data generator for raw cspec data
set(mgcncsgenjson_SRC mgcncsgen/mgcncsgenjson.cpp mgcncsgen/JsonArgs.cpp
    ../../dataformats/multigrid/src/RunSpecParse.cpp
    ../../dataformats/multigrid/src/jsoncpp.cpp)
set(mgcncsgenjson_INC mgcncsgen/JsonArgs.h)
create_executable(mgcncsgenjson "")

enable_testing()
#
# Multi Grid Tests
#
set(MGCNCSChanConvTest_SRC mgcncs/ChanConvTest.cpp mgcncs/ChanConv.cpp)
create_test_executable(MGCNCSChanConvTest "")

set(MGCNCSDataTest_SRC mgcncs/DataTest.cpp mgcncs/Data.cpp mgcncs/ChanConv.cpp)
set(MGCNCSDataTest_INC mgcncs/Data.h mgcncs/ChanConv.h mgcncs/TestData.h mgcncs/Geometry.h)
create_test_executable(MGCNCSDataTest "")

#set(CSPECEventTest_SRC cspec/CSPECEventTest.cpp)
#set(CSPECEventTest_INC cspec/CSPECEvent.h)
#create_test_executable(CSPECEventTest "")

if(${CMAKE_COMPILER_IS_GNUCXX})
    set(CalibrationFileTest_LIB_SRC mgcncs/CalibrationFileTest.cpp ../test/SyscallWrappers.cpp)
    add_library(CalibrationFileTest_LIB STATIC ${CalibrationFileTest_LIB_SRC})
    set_target_properties(CalibrationFileTest_LIB PROPERTIES COMPILE_FLAGS "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")

    set(CalibrationFileTest_SRC mgcncs/CalibrationFile.cpp)
    set(CalibrationFileTest_INC mgcncs/CalibrationFile.h)
    set(CalibrationFileTest_LINK CalibrationFileTest_LIB ${GTEST_BOTH_LIBRARIES})
    create_test_executable(CalibrationFileTest "${CalibrationFileTest_LINK}")
    add_compile_flags(CalibrationFileTest "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
    add_linker_flags(CalibrationFileTest "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
endif()
