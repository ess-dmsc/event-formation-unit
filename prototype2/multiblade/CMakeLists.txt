cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
#enable_testing()

#include_directories(.. ../..)

#set(MBtestcounter_SRC
#        mbcaen/multiBladeEventBuilder.cpp
#        mbcaen/multiBladeEventBuilderCounterTest.cpp
#        )

#set(MBtestcluster_SRC
#        mbcaen/multiBladeEventBuilder.cpp
#        mbcaen/multiBladeEventBuilderClusteringTest.cpp
#        )

#set(MBtest_INC
#        ../test/TestBase.h
#        mbcaen/multiBladeEventBuilder.h
#        mbcaen/multiBladeTestData.h
#        )

#set(MBtestcounter_INC ${MBtest_INC})
#set(MBtestcluster_INC ${MBtest_INC})

#set(MBtest_LINK
#        ${GTEST_BOTH_LIBRARIES}
#        Threads::Threads
#        )

#add_library(MBtestcounter_LIB STATIC ${MBtestcounter_SRC})

#create_test_executable(MBtestcounter "")
