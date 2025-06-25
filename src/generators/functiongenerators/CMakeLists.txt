# Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
#=============================================================================


#=============================================================================
# Function Generators tests
#=============================================================================

set(DistributionGeneratorTest_INC
  DistributionGenerator.h
  )
set(DistributionGeneratorTest_SRC
  test/DistributionGeneratorTest.cpp
  DistributionGenerator.cpp
  )
create_test_executable(DistributionGeneratorTest)

set(LinearGeneratorTest_INC
  LinearGenerator.h
  )
set(LinearGeneratorTest_SRC
  test/LinearGeneratorTest.cpp
  )
create_test_executable(LinearGeneratorTest)
