# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Kafka related classes and tests
#=============================================================================
include_directories(.)

# set(Serializer_SRC
#   DA00Tables.h
#   BinAggregation.h
#   AbstractSerializer.h
#   Frame1DHistogramSerializer.h)

# add_library(Serializer OBJECT
#   ${Serializer_SRC}
#   )

# enable_coverage_flags(EssReadoutLib)

set(Frame1DHistogramSerializerTest_SRC
  test/Frame1DHistogramSerializerTest.cpp
  )
  
create_test_executable(Frame1DHistogramSerializerTest)
