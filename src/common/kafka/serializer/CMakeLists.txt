# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Kafka related classes and tests
#=============================================================================

include_directories(.)

set(DA00HistogramSerializerTest_SRC
  test/DA00HistogramSerializerTest.cpp
  )

create_test_executable(DA00HistogramSerializerTest)
