#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

set(pipeline_common_source
    GdGemBase.cpp
    NMXConfig.cpp
    nmx/Cluster.cpp
    nmx/Event.cpp
    nmx/Hit.cpp
    ../common/Hists.cpp
    ../common/HistSerializer.cpp
    nmx/TrackSerializer.cpp
    nmx/AbstractBuilder.cpp
    clustering/HitsQueue.cpp
    clustering/HitSorter.cpp
    clustering/ClusterMatcher.cpp
    clustering/DoroClusterer.cpp
    srs/SRSMappings.cpp
    srs/SRSTime.cpp
    vmm3/ParserVMM3.cpp
    vmm3/BuilderVMM3.cpp
    vmm3/CalibrationFile.cpp  # Also works for VMM2
)

set(pipeline_common_inc
    GdGemBase.h
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
    clustering/HitsQueue.h
    clustering/HitSorter.h
    clustering/ClusterMatcher.h
    clustering/DoroClusterer.h
    clustering/HitContainer.h
    clustering/AbstractClusterer.h
    srs/SRSMappings.h
    srs/SRSTime.h
    vmm3/ParserVMM3.h
    vmm3/BuilderVMM3.h
    vmm3/CalibrationFile.h # also works for VMM2
)

set(gdgem_SRC GdGemReadout.cpp ${GdGemBase_SRC} ${pipeline_common_source})
set(gdgem_INC ${pipeline_common_inc})
create_module(gdgem)

#
# Unit Tests
#
set(NMXConfigTest_INC NMXConfig.h srs/SRSTime.h srs/SRSMappings.h vmm3/CalibrationFile.cpp)
set(NMXConfigTest_SRC NMXConfigTest.cpp NMXConfig.cpp srs/SRSTime.cpp srs/SRSMappings.cpp vmm3/CalibrationFile.cpp)
create_test_executable(NMXConfigTest)

set(GdGemBaseTest_INC
    ${pipeline_common_inc}
    ../../libs/include/Socket.h
    ../../libs/include/TSCTimer.h
    ../adc_readout/test/TestUDPServer.h)
set(GdGemBaseTest_SRC
    GdGemBaseTest.cpp
    ${pipeline_common_source}
    ../../libs/source/Socket.cpp
    ../../libs/source/TSCTimer.cpp
    ../adc_readout/test/TestUDPServer.cpp)
create_test_executable(GdGemBaseTest)

#=============================================================================
# Component tests
#=============================================================================



add_subdirectory(nmx)
add_subdirectory(srs)
add_subdirectory(vmm3)
add_subdirectory(clustering)
add_subdirectory(generators)
