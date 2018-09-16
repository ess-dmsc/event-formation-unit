#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

set(gdgem_SRC
    gdgem.cpp
    NMXConfig.cpp
    nmx/ReadoutFile.cpp
    nmx/Cluster.cpp
    nmx/Event.cpp
    nmx/Hit.cpp
    nmx/HitFile.cpp
    ../common/Hists.cpp
    ../common/HistSerializer.cpp
    nmx/TrackSerializer.cpp
    nmx/AbstractBuilder.cpp
    generators/BuilderAPV.cpp
    generators/BuilderHits.cpp
    clustering/HitsQueue.cpp
    clustering/HitSorter.cpp
    clustering/ClusterMatcher.cpp
    clustering/DoroClusterer.cpp
    srs/SRSMappings.cpp
    srs/SRSTime.cpp
    vmm2/ParserVMM2.cpp
    vmm2/BuilderVMM2.cpp
    vmm3/ParserVMM3.cpp
    vmm3/BuilderVMM3.cpp
    vmm3/CalibrationFile.cpp  # Also works for VMM2
    )
set(gdgem_INC
    NMXConfig.h
    nmx/Cluster.h
    nmx/Event.h
    nmx/Hit.h
    ../common/Hists.h
    nmx/Readout.h
    nmx/ReadoutFile.h
    ../common/HistSerializer.h
    nmx/TrackSerializer.h
    nmx/AbstractBuilder.h
    generators/BuilderAPV.h
    generators/BuilderHits.h
    clustering/HitsQueue.h
    clustering/HitSorter.h
    clustering/ClusterMatcher.h
    clustering/DoroClusterer.h
    clustering/HitContainer.h
    clustering/AbstractClusterer.h
    srs/SRSMappings.h
    srs/SRSTime.h
    vmm2/ParserVMM2.h
    vmm2/BuilderVMM2.h
    vmm3/ParserVMM3.h
    vmm3/BuilderVMM3.h
    vmm3/CalibrationFile.h # also works for VMM2
    )
create_module(gdgem)

#
# Unit Tests
#
set(NMXConfigTest_INC NMXConfig.h srs/SRSTime.h srs/SRSMappings.h vmm3/CalibrationFile.cpp)
set(NMXConfigTest_SRC NMXConfigTest.cpp NMXConfig.cpp srs/SRSTime.cpp srs/SRSMappings.cpp vmm3/CalibrationFile.cpp)
create_test_executable(NMXConfigTest)

#=============================================================================
# Component tests
#=============================================================================



add_subdirectory(nmx)
add_subdirectory(srs)
add_subdirectory(vmm2)
add_subdirectory(vmm3)
add_subdirectory(clustering)
add_subdirectory(generators)
