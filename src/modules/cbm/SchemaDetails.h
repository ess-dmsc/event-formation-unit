// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief SchemaDetails definitions of schema to serializer
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/kafka/EV44Serializer.h>
#include <common/kafka/serializer/DA00HistogramSerializer.h>
#include <modules/cbm/SchemaType.h>

#include <memory>
#include <tuple>

namespace cbm {

/// \brief Structure used for storing the serializer information for a specific schema type. 
/// Field Schema indicates the serializer that must be used.  
class SchemaDetails {
public:
  //Current serializer used for CBM's
  using DA00Serializer_t = fbserializer::HistogramSerializer<int32_t, int32_t, uint64_t>;
  using EV44Serializer_t = EV44Serializer;
  /// SchemaSerializers used in this class are linked to SchemaType::Type enum
  /// values.
  /// \warning Tuple order MUST match SchemaType::Types enum values exactly:
  ///   Index 0 → DA00 serializer (SchemaType::DA00 = 0)
  ///   Index 1 → EV44 serializer (SchemaType::EV44 = 1)
  /// Adding new types requires updating BOTH enum AND tuple in matching
  /// positions.
  using SchemaSchemaSerializers_t = std::tuple<
                              std::unique_ptr<DA00Serializer_t>, 
                              std::unique_ptr<EV44Serializer_t>>;  

  /// \brief Return the wrapped schema type stored. Values are determine at construction 
  /// time
  /// \return Schema type enum value for this object
  SchemaType GetSchema() const;

  /// \brief Return a serializer object. Template parameter point to element
  /// in SchemaSchemaSerializers_t. Please note that that template parameter must 
  /// point to correct index in SchemaSchemaSerializers_t or else a null pointer is
  /// returned to a different object type.
  ///
  /// \return Pointer to Serializer object linked to template parameters. If wrong enum value
  /// is used to get serializer a nullptr to a different object type is returned.
  template <int Index_t>
  auto GetSerializer() const ->
    typename std::tuple_element_t<Index_t, SchemaSchemaSerializers_t>::element_type*;

  /// \brief Constructor for HistogramBuilder or DA00 serializer
  ///
  /// \param Schema   Schema Type to be used for extracting correct serializer
  /// \param Source   Data source
  /// \param Period   Duration one frame in the specified units
  /// \param BinCount Number of used bins
  /// \param DataUnit Bin unit
  /// \param AggregatedFrames Number of pulses to aggregate before send
  /// \param Callback Producer callback function
  /// \param BinOffset Bin offset 
  /// \param AggFunc  Function used to aggregate bin data
  explicit SchemaDetails(SchemaType Schema, 
              const std::string &Source, 
              time_t Period, 
              time_t BinCount, 
              const std::string &DataUnit,
              uint8_t AggregatedFrames, 
              ProducerCallback Callback, 
              int32_t BinOffset,
              essmath::VectorAggregationFunc<int32_t> AggFunc);

  /// \brief Constructor for EV44 serializer
  ///
  /// \param Schema   Schema Type to be used for extracting correct serializer
  /// \param MaxArrayLength maximum number of events
  /// \param Source value for source_name field
  /// \param Callback Producer callback function
  explicit SchemaDetails(SchemaType Schema, size_t MaxArrayLength, const std::string &Source,
                 ProducerCallback Callback);              

  /// \brief Constructor for HistogramBuilder or EV44 serializer
  ///
  /// \param Serializer   DA00 serializer
  explicit SchemaDetails(SchemaType Schema, std::unique_ptr<DA00Serializer_t> &&Serializer);

  /// \brief Constructor for HistogramBuilder or DA00 serializer
  ///
  /// \param Serializer   EV00 serializer
  explicit SchemaDetails(SchemaType Schema, std::unique_ptr<EV44Serializer_t> &&Serializer);

private:

  SchemaType SchemaSerializerType{};
  SchemaSchemaSerializers_t SchemaSerializers; 
};

inline SchemaDetails::SchemaDetails(SchemaType Schema, std::unique_ptr<DA00Serializer_t> &&Serializer)
  : SchemaSerializerType{Schema}
  , SchemaSerializers{std::move(Serializer), nullptr} {

  if (SchemaSerializerType != SchemaType::DA00) {
    throw std::runtime_error(
        "Invalid Schema type used for DA00 serializer in schema details");
  }
}

inline SchemaDetails::SchemaDetails(SchemaType Schema, std::unique_ptr<EV44Serializer_t> &&Serializer)
  : SchemaSerializerType{Schema}
  , SchemaSerializers{nullptr, std::move(Serializer)} {

  if (SchemaSerializerType != SchemaType::EV44) {
    throw std::runtime_error(
        "Invalid Schema type used for EV44 serializer in schema details");
  }
}

inline SchemaDetails::SchemaDetails(SchemaType Schema, const std::string &Source, time_t Period, 
  time_t BinCount, const std::string &DataUnit, uint8_t AggregatedFrames, 
  ProducerCallback Callback, int32_t BinOffset, essmath::VectorAggregationFunc<int32_t> AggFunc)
    : SchemaDetails{Schema,
      std::make_unique<DA00Serializer_t>(
        Source, Period, BinCount, DataUnit,  AggregatedFrames, 
        Callback, BinOffset, AggFunc)} {}

inline SchemaDetails::SchemaDetails(SchemaType Schema, size_t MaxArrayLength, const std::string &Source, 
  ProducerCallback Callback)
    : SchemaDetails{Schema, 
      std::make_unique<EV44Serializer_t>(MaxArrayLength, Source, Callback)} {}

inline SchemaType SchemaDetails::GetSchema() const {
    return SchemaSerializerType;
}

template <int Index_t>
inline auto SchemaDetails::GetSerializer() const ->
  typename std::tuple_element_t<Index_t, SchemaDetails::SchemaSchemaSerializers_t>::element_type* {

  static_assert(Index_t >= SchemaType::MIN && Index_t <= SchemaType::MAX, "Undefined serializer is request in GetSerializer.");
  // Get the unique_ptr from the tuple and return the raw pointer
  return std::get<Index_t>(SchemaSerializers).get();
}

} // namespace cbm