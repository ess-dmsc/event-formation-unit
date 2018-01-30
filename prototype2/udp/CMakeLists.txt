cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

include_directories(.. ../..)

#
# Detector Pipeline Plugins
#

# UDP receiver 'detector'
set(udp_SRC udp.cpp)
create_module(udp "")
set_target_properties(udp PROPERTIES COMPILE_FLAGS "-fno-strict-aliasing")
