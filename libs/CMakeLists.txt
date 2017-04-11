cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

include_directories(..)

set(library_SRC source/Socket.cpp source/Timer.cpp source/Thread.cpp source/TSCTimer.cpp)
set(library_INC include/Socket.h include/Timer.h include/Thread.h include/Counter.h include/TSCTimer.h include/Counter.h include/StatCounter.h)

add_library(eventlib ${library_SRC} ${library_INC})

find_package(Threads)

if(${GTEST_FOUND})
    enable_testing()
    add_executable(CounterTest test/Counter/testmain.cpp test/Counter/CounterTest.cpp)
    target_link_libraries(CounterTest ${CMAKE_THREAD_LIBS_INIT} ${GTEST_BOTH_LIBRARIES})
    set_target_properties(CounterTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/unit_tests")

    add_executable(StatCounterTest test/Counter/testmain.cpp test/Counter/StatCounterTest.cpp)
    target_link_libraries(StatCounterTest ${CMAKE_THREAD_LIBS_INIT} ${GTEST_BOTH_LIBRARIES})
    set_target_properties(StatCounterTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/unit_tests")
    
    add_test(NAME CounterTest COMMAND CounterTest "--gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/CounterTesttest.xml")
    add_test(NAME StatCounterTest COMMAND StatCounterTest "--gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/StatCounterTesttest.xml")
    
    add_custom_target(runtest_libs COMMAND ${CMAKE_CTEST_COMMAND} -V DEPENDS StatCounterTest CounterTest)
endif()