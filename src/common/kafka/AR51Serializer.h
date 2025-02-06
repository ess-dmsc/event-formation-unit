// Copyright (C) 2023 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief flatbuffer serialization into ar52 schema
///
/// See https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#pragma once

#include "Producer.h"
#include "ar51_readout_data_generated.h"


class AR51Serializer {
public:
  /// \brief creates ar51 flat buffer serializer
  /// \param source_name value for source_name field
  /// \param Callback
  AR51Serializer(const std::string &SourceName, ProducerCallback Callback = {});

  /// \brief adds event, if maximum count is exceeded, sends data using the
  /// producer callback \param time time of event in relation to pulse time
  /// \param pixl id of pixel as defined by logical geometry mapping
  /// \returns bytes transmitted, if any
  // size_t addEvent(int32_t Time, int32_t Pixel);


  /// \brief serializes and sends to producer
  /// \returns bytes transmitted
  size_t produce();

  // \todo make private?
  /// \brief serializes buffer
  /// \returns reference to internally stored buffer
  nonstd::span<const uint8_t> & serialize(uint8_t * Data, int DataLength);

public:
  // All of this is the flatbuffer
  flatbuffers::FlatBufferBuilder FBBuilder;

  nonstd::span<const uint8_t> FBuffer;
  std::string Source{""};
  ProducerCallback ProduceFunctor;
  int64_t SeqNum{0};
  int64_t TxBytes{0};
};
