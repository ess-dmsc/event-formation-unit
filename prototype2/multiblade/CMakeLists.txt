cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
enable_testing()
#
# Multiblade Tests
#
include_directories(..)
set(MBDummyTest_SRC mbcaen/DummyClassTest.cpp mbcaen/DummyClass.cpp)
set(MBDummyTest_INC ../test/TestBase.h mbcaen/DummyClass.h)
create_test_executable(MBDummyTest "")
