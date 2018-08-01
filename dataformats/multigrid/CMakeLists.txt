include_directories(
  inc
  ../../prototype2
)

add_executable(batchreader
  src/batchreader.cpp
  src/Analyse.cpp
  src/Args.cpp
  src/MapFile.cpp
  src/PeakFinder.cpp
  src/RunSpecParse.cpp
  src/jsoncpp.cpp
  ../../prototype2/common/DataSave.cpp
  ../../prototype2/multigrid/mgcncs/ChanConv.cpp
  ../../prototype2/multigrid/mgcncs/CalibrationFile.cpp
  ../../prototype2/multigrid/mgcncs/DataParser.cpp
  )

target_link_libraries(batchreader PRIVATE efu_common)

add_executable(cncsread src/cncsread.cpp)

add_executable(genpixids
  src/genpixids.cpp
  ../../prototype2/common/DataSave.cpp
  ../../prototype2/multigrid/mgcncs/ChanConv.cpp
  ../../prototype2/multigrid/mgcncs/CalibrationFile.cpp
  )
target_link_libraries(genpixids PRIVATE efu_common)

#
# Install Files
#
install(TARGETS cncsread genpixids batchreader DESTINATION tools)

#
# Unit tests
#
set(RunSpecTest_SRC inc/RunSpecTest.cpp)
set(RunSpecTest_INC inc/RunSpec.h)
create_test_executable(RunSpecTest)

set(RunSpecParseTest_SRC src/RunSpecParse.cpp inc/RunSpecParseTest.cpp ../../prototype2/common/DataSave.cpp)
set(RunSpecParseTest_INC inc/RunSpec.h inc/RunSpecParse.h)
create_test_executable(RunSpecParseTest)

set(PeakFinderTest_SRC
    src/test/PeakFinderTest.cpp
    src/PeakFinder.cpp)
set(PeakFinderTest_INC)
create_test_executable(PeakFinderTest)

set(HistogramTest_SRC
    src/test/HistogramTest.cpp)
set(HistogramTest_INC)
create_test_executable(HistogramTest)
