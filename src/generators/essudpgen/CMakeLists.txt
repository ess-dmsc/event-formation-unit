# Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
#=============================================================================


#=============================================================================
# TofDistributionTest
#=============================================================================

set(TofDistributionTest_INC
  TofDistribution.h
  )
set(TofDistributionTest_SRC
  TofDistributionTest.cpp
  TofDistribution.cpp
  )
create_test_executable(TofDistributionTest)
