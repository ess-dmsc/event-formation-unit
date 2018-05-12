#=============================================================================
# APV2VMM Generator (h5)
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

set(BuilderAPVTest_SRC
    ../nmx/EventletFile.cpp
    ../nmx/Event.cpp
    ../nmx/Cluster.cpp
    ../nmx/Clusterer.cpp
    ../nmx/Hists.cpp
    ../nmx/AbstractBuilder.cpp
    BuilderAPVTest.cpp
    BuilderAPV.cpp
    )
set(BuilderAPVTest_INC
    ../nmx/EventletFile.h
    ../nmx/Clusterer.h
    ../nmx/Cluster.h
    ../nmx/Event.h
    ../nmx/Eventlet.h
    ../nmx/Hists.h
    BuilderAPV.h
    )
create_test_executable(BuilderAPVTest)

#=============================================================================
# Raw evenlet generator (h5)
#=============================================================================

set(nmxgen_eventlets_SRC
    ../nmx/EventletFile.cpp
    ../nmx/Eventlet.cpp
    nmxgen_eventlets.cpp
    ReaderEventlets.cpp
    NMXArgs.cpp
    )
set(nmxgen_eventlets_INC
    ../nmx/EventletFile.h
    ../nmx/Eventlet.h
    ReaderEventlets.h
    NMXArgs.h
    )
create_executable(nmxgen_eventlets)

#=============================================================================
# PCAP generator
#=============================================================================

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
