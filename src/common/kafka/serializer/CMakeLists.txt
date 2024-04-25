# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Kafka related classes and tests
#=============================================================================

include_directories(.)

set(HistogramSerializerTest_SRC
  serializer/test/HistogramSerializerTest.cpp
  )

create_test_executable(Frame1DHistogramSerializerTest)
