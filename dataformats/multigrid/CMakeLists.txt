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

add_executable(cncsread src/cncsread.cpp)

add_executable(genpixids
  src/genpixids.cpp
  ../../prototype2/common/DataSave.cpp
  ../../prototype2/multigrid/mgcncs/ChanConv.cpp
  ../../prototype2/multigrid/mgcncs/CalibrationFile.cpp
  )

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

set(PeakFinderTest_SRC
    src/test/PeakFinderTest.cpp
    src/PeakFinder.cpp)
set(PeakFinderTest_INC)
create_test_executable(PeakFinderTest)

set(HistogramTest_SRC
    src/test/HistogramTest.cpp)
set(HistogramTest_INC)
create_test_executable(HistogramTest)
