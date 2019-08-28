set(MGFilterTest_SRC
  FilterTest.cpp
  )
set(MGFilterTest_LIB
  MgGeometry
  )
create_test_executable(MGFilterTest)

set(MGChannelMappingsTest_SRC
  ChannelMappingsTest.cpp
  )
set(MGChannelMappingsTest_LIB
  MgGeometry
  )
create_test_executable(MGChannelMappingsTest)

set(MGSeqMappingsTest_SRC
  MGSeqMappingsTest.cpp
  )
set(MGSeqMappingsTest_LIB
  MgGeometry
  )
create_test_executable(MGSeqMappingsTest)

set(MG24MappingsTest_SRC
  MG24MappingsTest.cpp
  )
set(MG24MappingsTest_LIB
  MgGeometry
  )
create_test_executable(MG24MappingsTest)

set(MGDetectorMappingsTest_SRC
  DetectorMappingsTest.cpp
  )
set(MGDetectorMappingsTest_LIB
  MgGeometry
  )
create_test_executable(MGDetectorMappingsTest)
