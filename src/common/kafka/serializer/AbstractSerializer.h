// Copyright (C) 2024 European Spallation Source, see LICENSE file
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

#pragma once

#include <chrono>
#include <common/kafka/Producer.h>
#include <flatbuffers/flatbuffers.h>
#include "common/time/ESSTime.h"

namespace fbserializer {

using namespace std::chrono;
using namespace esstime;

/// \struct SerializerStats
///
/// \brief Structure to hold statistics related to serialization.
///
struct SerializerStats {
  int64_t ProduceCalled{
      0}; ///< Number of times the produce() function has been called.
};

/// \class AbstractSerializer
///
/// \brief Abstract base class for serializers.
///
class AbstractSerializer {

  const ProducerCallback
      ProduceCallback; ///< Callback function for producing serialized data.

  SerializerStats &Stats; ///< Reference to the ProduceCalled statistic.

protected:
  esstime::ESSReferenceTime ReferenceTime{
      esstime::ESSReferenceTime()};   ///< Reference time for serialization.
  flatbuffers::DetachedBuffer Buffer; ///< Buffer to hold serialized data.

  /// \brief Constructs an AbstractSerializer object with the given callback and
  /// statistics.
  ///
  /// \param Callback The callback function for producing serialized data.
  /// \param Stats The statistics object to track serialization statistics.
  ///
  AbstractSerializer(const ProducerCallback Callback, SerializerStats &Stats);

  /// \brief Copy constructor.
  ///
  /// \param other The AbstractSerializer object to copy.
  ///
  AbstractSerializer(const AbstractSerializer &other);

  /// \brief Pure virtual function to serialize data.
  ///
  virtual void serialize() = 0;

public:
  /// \brief Destructor.
  ///
  virtual ~AbstractSerializer() = default;

  /// \brief Produces serialized data.
  ///
  /// This function increments the ProduceCalled statistic, gets the current
  /// hardware clock, calls the serialize() function, and invokes the
  /// ProduceCallback with the serialized data and the current hardware clock.
  ///
  void produce();

  /// \brief Sets the reference time for serialization.
  ///
  /// \param Time The reference time to set.
  ///
  void setReferenceTime(const ESSTime &Time);
};

} // namespace fbserializer