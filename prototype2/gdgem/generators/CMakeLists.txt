#=============================================================================
# Data generators
#=============================================================================

set(nmxgen_apv_SRC
    ../nmx/Eventlet.cpp
    nmxgen_apv.cpp
    NMXArgs.cpp
    ReaderAPV.cpp
    )
set(nmxgen_apv_INC
    ../nmx/Eventlet.h
    NMXArgs.h
    ReaderAPV.h
    )
create_executable(nmxgen_apv)

set(EventletBuilderAPVTest_SRC
    ../nmx/EventNMX.cpp
    ../nmx/Clusterer.cpp
    ../nmx/Hists.cpp
    ../nmx/AbstractEventletBuilder.cpp
    EventletBuilderAPVTest.cpp
    EventletBuilderAPV.cpp
    )
set(EventletBuilderAPVTest_INC
    ../nmx/Clusterer.h
    ../nmx/EventNMX.h
    ../nmx/Eventlet.h
    ../nmx/Hists.h
    EventletBuilderAPV.h
    )
create_test_executable(EventletBuilderAPVTest)

find_package(PCAP)
if(PCAP_FOUND)
  set(nmxgen_pcap_SRC
      nmxgen_pcap.cpp
      ReaderPcap.cpp
      NMXArgs.cpp
      )
  set(nmxgen_pcap_INC
      ReaderPcap.h
      NMXArgs.h
      )
  set(nmxgen_pcap_LIB
      ${PCAP_LIBRARY}
      )
  create_executable(nmxgen_pcap)
else()
  message(STATUS "*** Unable to compile gennmxpcap as libpcap was not found.")
endif()
