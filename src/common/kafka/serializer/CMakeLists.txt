# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Kafka related classes and tests
#=============================================================================
include_directories(.)

set(Serializer_SRC
  DA00Serializer.cpp)

set(Serializer_INC
  builders/DA00Tables.h
  builders/DA00FlatbufferBuilders.h
  AbstractSerializer.h
  DA00Serializer.h)

add_library(Serializer OBJECT
  ${Serializer_SRC}
  ${Serializer_INC}
  )

enable_coverage_flags(EssReadoutLib)

set(DA00Test_SRC
  test/DA00SerializerTest.cpp
  )
  
create_test_executable(DA00Test)
