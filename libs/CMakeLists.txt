cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

add_definitions(-std=c++11 -pthread) #-v = verbose

include_directories(..)

set(library_SRC source/Socket.cpp source/Timer.cpp source/Thread.cpp source/TSCTimer.cpp)
set(library_INC include/Socket.h include/Timer.h include/Thread.h include/Counter.h include/TSCTimer.h include/Counter.h include/StatCounter.h)

add_library(eventlib ${library_SRC} ${library_INC})

if(GTEST_FOUND)    
    add_executable(CounterTest test/Counter/testmain.cpp test/Counter/CounterTest.cpp)
    target_link_libraries(CounterTest ${GTEST_BOTH_LIBRARIES})

    add_executable(StatCounterTest test/Counter/testmain.cpp test/Counter/StatCounterTest.cpp)
    target_link_libraries(StatCounterTest ${GTEST_BOTH_LIBRARIES})
endif()