// Copyright (C) 2022 - 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/serializer/DA00HistogramSerializer.h>
#include <modules/cbm/Counters.h>
#include <modules/cbm/geometry/Config.h>

namespace cbm {

/// @class A class template representing a map of serializers.
///
/// This class template provides a map-like container for storing and accessing
/// serializers. It allows adding and retrieving serializers based on a given
/// FEN id and Channel.
///
/// @tparam T The type of the serializers to be stored.
template <typename T> class SerializerMap {
public:
  /// Default constructor.
  SerializerMap() = default;

  /// Adds a serializer to the map.
  ///
  /// This function adds a serializer to the map based on the given FEN and
  /// Channel. The serializer is moved into the map, so the original object is
  /// no longer valid after this function is called.
  ///
  /// @param FEN The File Extension Number.
  /// @param Channel The channel number.
  /// @param value The serializer to be added.
  void add(int FEN, int Channel, std::unique_ptr<T> &value) {
    int index = FEN * MaxFen + Channel;
    Serializers[index] = std::move(value);
  }

  /// Retrieves a serializer from the map.
  ///
  /// This function retrieves a reference to a serializer's unique ptr from the
  /// map based on the given FEN and Channel.
  ///
  /// @param FEN The File Extension Number.
  /// @param Channel The channel number.
  /// @return A reference to the serializer.
  /// @throws std::out_of_range if the serializer does not exist in the map.
  const std::unique_ptr<T> &get(int FEN, int Channel) const {
    int index = FEN * MaxFen + Channel;
    return Serializers.at(index);
  }

  /// Retrieves all the serializers in the map.
  ///
  /// This function returns a reference to the map containing all the
  /// serializers.
  ///
  /// @return A reference to the map of serializers.
  std::map<int, std::unique_ptr<T>> &getAllSerializers() { return Serializers; }

private:
  const int MaxFen = Config::MaxFEN;
  std::map<int, std::unique_ptr<T>> Serializers;
};

/// A class representing the CbmBase detector.
///
/// This class inherits from the Detector base class and provides additional
/// functionality specific to the CbmBase detector.
class CbmBase : public Detector {
public:
  /// Constructor.
  ///
  /// Constructs a CbmBase object with the given settings.
  ///
  /// @param settings The settings for the CbmBase detector.
  CbmBase(BaseSettings const &settings);

  /// Destructor.
  ~CbmBase() = default;

  /// The processing thread function.
  ///
  /// This function is responsible for the processing logic of the CbmBase
  /// detector.
  void processing_thread();

  /// Struct representing counters.
  ///
  /// This struct holds counters for various operations related to the CbmBase
  /// detector.
  struct Counters Counters {};

protected:
  SerializerMap<EV44Serializer> EV44SerializerPtrs;
  SerializerMap<fbserializer::HistogramSerializer<int32_t>>
      HistogramSerializerPtrs;
};

} // namespace cbm
