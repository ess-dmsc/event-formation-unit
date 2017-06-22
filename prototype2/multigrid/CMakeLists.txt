#
# Detector Pipeline Plugins
#

# Multi Grid demonstrator detector
set(mgcncs_SRC multigrid/mgcncs.cpp multigrid/mgcncs/Data.cpp multigrid/mgcncs/ChanConv.cpp)
set(mgcncs_INC multigrid/mgcncs/Data.h multigrid/mgcncs/ChanConv.h multigrid/mgcncs/Geometry.h)
create_module(mgcncs "")


# Two-thread implementation of Multi Grid demonstrator detector
set(mgcncs2_SRC multigrid/mgcncs2.cpp multigrid/mgcncs/Data.cpp multigrid/mgcncs/ChanConv.cpp)
set(mgcncs2_INC multigrid/mgcncs/Data.h multigrid/mgcncs/ChanConv.h multigrid/mgcncs/Geometry.h)
create_module(mgcncs2 "")



#
# Data Generators
#

# Generates dummy 'CSPEC' data matching the Multi Grid demonstrator
set(mgcncsgen_SRC multigrid/mgcncsgen/mgcncsgen.cpp multigrid/mgcncsgen/DGArgs.cpp multigrid/mgcncs/Data.cpp)
set(mgcncsgen_INC multigrid/mgcncsgen/DGArgs.h multigrid/mgcncs/Data.h)
create_executable(mgcncsgen  "")

#
set(mgcncsgenfile_SRC multigrid/mgcncsgen/mgcncsgenfile.cpp multigrid/mgcncsgen/DGArgs.cpp)
set(mgcncsgenfile_INC multigrid/mgcncsgen/DGArgs.h)
create_executable(mgcncsgenfile "")

# Data generator for raw cspec data
set(mgcncsgenjson_SRC multigrid/mgcncsgen/mgcncsgenjson.cpp multigrid/mgcncsgen/JsonArgs.cpp
    ../dataformats/multigrid/src/RunSpecParse.cpp
    ../dataformats/multigrid/src/jsoncpp.cpp)
set(mgcncsgenjson_INC multigrid/mgcncsgen/JsonArgs.h)
create_executable(mgcncsgenjson "")


#
# Multi Grid Tests
#
set(MGCNCSChanConvTest_SRC multigrid/mgcncs/ChanConvTest.cpp multigrid/mgcncs/ChanConv.cpp)
create_test_executable(MGCNCSChanConvTest "")

set(MGCNCSDataTest_SRC multigrid/mgcncs/DataTest.cpp multigrid/mgcncs/Data.cpp multigrid/mgcncs/ChanConv.cpp)
set(MGCNCSDataTest_INC multigrid/mgcncs/Data.h multigrid/mgcncs/ChanConv.h multigrid/mgcncs/TestData.h multigrid/mgcncs/Geometry.h)
create_test_executable(MGCNCSDataTest "")

#set(CSPECEventTest_SRC cspec/CSPECEventTest.cpp)
#set(CSPECEventTest_INC cspec/CSPECEvent.h)
#create_test_executable(CSPECEventTest "")

if(${CMAKE_COMPILER_IS_GNUCXX})
    set(CalibrationFileTest_LIB_SRC multigrid/mgcncs/CalibrationFileTest.cpp test/SyscallWrappers.cpp)
    add_library(CalibrationFileTest_LIB STATIC ${CalibrationFileTest_LIB_SRC})
    set_target_properties(CalibrationFileTest_LIB PROPERTIES COMPILE_FLAGS "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")

    set(CalibrationFileTest_SRC multigrid/mgcncs/CalibrationFile.cpp)
    set(CalibrationFileTest_INC multigrid/mgcncs/CalibrationFile.h)
    set(CalibrationFileTest_LINK CalibrationFileTest_LIB ${GTEST_BOTH_LIBRARIES})
    create_test_executable(CalibrationFileTest "${CalibrationFileTest_LINK}")
    add_compile_flags(CalibrationFileTest "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
    add_linker_flags(CalibrationFileTest "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
endif()
