#=============================================================================
# EFU Tests
#=============================================================================

#
set(HwCheckTest_INC
  ../HwCheck.h
)
set(HwCheckTest_SRC
  ../HwCheck.cpp
  HwCheckTest.cpp
)
create_test_executable(HwCheckTest)

#
set(ServerTest_INC
  ../Server.h
  ../Parser.h
)
set(ServerTest_SRC
  ServerTest.cpp
)
create_test_executable(ServerTest)

#
set(ParserTest_SRC
  ParserTest.cpp
)
set(ParserTest_INC
  ../Parser.h
  )
create_test_executable(ParserTest)

#
set(LauncherTest_SRC
  LauncherTest.cpp
)
set(LauncherTest_INC
  ../Launcher.h
  )

set(DataPipelineTest_SRC
    DataPipelineTest.cpp
)
set(DataPipelineTest_INC
  ../DataPipeline.h
  )

  set(DataPipelinePerfTest_SRC
  DataPipelinePerfTest.cpp
)
set(DataPipelinePerfTest_INC
../DataPipeline.h
)

create_test_executable(DataPipelinePerfTest)
  
create_test_executable(LauncherTest)
