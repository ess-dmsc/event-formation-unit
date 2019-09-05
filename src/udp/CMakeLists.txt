#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

# UDP receiver 'detector'
set(udp_SRC udp.cpp)
create_module(udp)
add_compile_flags(udp "-fno-strict-aliasing")
