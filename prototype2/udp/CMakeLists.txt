
#
# Detector Pipeline Plugins
#

# UDP receiver 'detector'
set(udp_SRC udp/udp.cpp)
create_module(udp "")
SET_TARGET_PROPERTIES(udp PROPERTIES COMPILE_FLAGS "-fno-strict-aliasing" )
