# Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
#=============================================================================


#=============================================================================
# CustomDistributionTest
#=============================================================================

set(CustomDistributionTest_INC
  CustomDistribution.h
  )
set(CustomDistributionTest_SRC
  CustomDistributionTest.cpp
  CustomDistribution.cpp
  )
create_test_executable(CustomDistributionTest)
