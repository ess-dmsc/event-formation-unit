// Copyright (C) 2022-2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Serialization objects for DA00 schema da00_DataArray and
/// da00_Variable tables
///
/// \author Gregory Tucker \date 2024-03-01
/// \link https://github.com/g5t/flatbuffer-histogram-generator
///
/// For flatbuffers schema see:
/// \link https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#include "common/kafka/Producer.h"
#include <cstddef>

class AbstractSerializer {

  ProducerCallback ProduceFunctor;

protected:
  AbstractSerializer(ProducerCallback callback) : ProduceFunctor(callback){};

public:
  virtual size_t produce() = 0;
};
