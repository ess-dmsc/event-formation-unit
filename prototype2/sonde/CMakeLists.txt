cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
enable_testing()
include_directories(..)
include_directories(../.. ${FLATBUFFERS_INCLUDE_DIR})

#
# Detector Pipeline Plugins
#

# IDEA based readout for SoNDe detector
set(sonde_SRC sonde.cpp ideas/Data.cpp)
set(sonde_INC ideas/Data.h)
create_module(sonde "")


#
# Tools and Data generators
#


#
# SoNDe Tests
#
set(IDEASDataTest_SRC
  ../../dataformats/multigrid/src/DataSave.cpp
  ideas/DataTest.cpp
  ideas/Data.cpp)
set(IDEASDataTest_INC
  ../../dataformats/multigrid/inc/DataSave.h
  ideas/Data.h
  ideas/TestData.h)
create_test_executable(IDEASDataTest "")


set(IDEASGeometryTest_SRC ideas/GeometryTest.cpp)
set(IDEASGeometryTest_INC Geometry.h)
create_test_executable(IDEASGeometryTest "")
