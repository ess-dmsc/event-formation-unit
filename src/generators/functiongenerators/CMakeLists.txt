# Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
#=============================================================================


#=============================================================================
# DistributionGeneratorTest
#=============================================================================

set(DistributionGeneratorTest_INC
  DistributionGenerator.h
  )
set(DistributionGeneratorTest_SRC
  DistributionGeneratorTest.cpp
  DistributionGenerator.cpp
  )
create_test_executable(DistributionGeneratorTest)
