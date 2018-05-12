#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

set(gdgem_SRC
    ../../dataformats/multigrid/src/jsoncpp.cpp
    gdgem.cpp
    NMXConfig.cpp
    nmx/Clusterer.cpp
    nmx/Cluster.cpp
    nmx/Event.cpp
    nmx/Eventlet.cpp
    nmx/Hists.cpp
    nmx/HistSerializer.cpp
    nmx/TrackSerializer.cpp
    nmx/AbstractEventletBuilder.cpp
    generators/EventletBuilderAPV.cpp
    generators/EventletBuilderEventlets.cpp
    srs/SRSMappings.cpp
    srs/SRSTime.cpp
    vmm2/ParserVMM2.cpp
    vmm2/EventletBuilderVMM2.cpp
    )
set(gdgem_INC
    NMXConfig.h
    nmx/Clusterer.h
    nmx/Cluster.h
    nmx/Event.h
    nmx/Eventlet.h
    nmx/Hists.h
    nmx/HistSerializer.h
    nmx/TrackSerializer.h
    nmx/AbstractEventletBuilder.h
    generators/EventletBuilderAPV.h
    generators/EventletBuilderEventlets.h
    srs/SRSMappings.h
    srs/SRSTime.h
    vmm2/ParserVMM2.h
    vmm2/EventletBuilderVMM2.h
    )
create_module(gdgem)

#=============================================================================
# Component tests
#=============================================================================

add_subdirectory(nmx)
add_subdirectory(srs)
add_subdirectory(vmm2)
add_subdirectory(vmm3)
add_subdirectory(clustering)

#=============================================================================
# Data generators
#=============================================================================

add_subdirectory(generators)
