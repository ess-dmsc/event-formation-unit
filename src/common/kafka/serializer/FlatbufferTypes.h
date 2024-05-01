/// Copyright (C) 2024 European Spallation Source, see LICENSE file
///===----------------------------------------------------------------------===//
///
/// This file contains serialization objects for the DA00 schema da00_DataArray
/// and da00_Variable tables. It provides classes for serializing and
/// deserializing data using FlatBuffers.
///
/// The classes in this file include:
/// - `da00flatbuffers::Variable`: Represents a variable with its name, type,
/// data, axes, shape, unit, label, and source.
/// - `da00flatbuffers::DataArray`: Represents a data array with its source
/// name, timestamp, and a vector of variables.
///
/// These classes are used to serialize and deserialize data using the DA00
/// schema defined in the FlatBuffers format. The serialization and
/// deserialization functions are provided in the `pack` and constructor methods
/// of the classes.
///
/// For more information about the DA00 schema and FlatBuffers, refer to the
/// following links:
/// - DA00 schema: https://github.com/ess-dmsc/streaming-data-types
/// - FlatBuffers: https://google.github.io/flatbuffers/
///
/// This code is part of the ESS EFU (Event Formation Unit) project, developed
/// by the European Spallation Source. For licensing information, see the
/// LICENSE file in the repository:
/// \link https://github.com/g5t/flatbuffer-histogram-generator
///
/// \author: Gregory Tucker
/// \date: 2024-03-01
/// \link: https://github.com/g5t/flatbuffer-histogram-generator
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>
#include <da00_dataarray_generated.h>
#include <numeric>
#include <optional>
#include <sstream>
#include "common/time/ESSTime.h"

namespace da00flatbuffers {

using namespace std::chrono;
using namespace esstime;

template <class> struct DataTypeTrait {
  static constexpr da00_dtype type = da00_dtype::none;
};
template <> struct DataTypeTrait<int8_t> {
  static constexpr da00_dtype type = da00_dtype::int8;
};
template <> struct DataTypeTrait<int16_t> {
  static constexpr da00_dtype type = da00_dtype::int16;
};
template <> struct DataTypeTrait<int32_t> {
  static constexpr da00_dtype type = da00_dtype::int32;
};
template <> struct DataTypeTrait<int64_t> {
  static constexpr da00_dtype type = da00_dtype::int64;
};
template <> struct DataTypeTrait<uint8_t> {
  static constexpr da00_dtype type = da00_dtype::uint8;
};
template <> struct DataTypeTrait<uint16_t> {
  static constexpr da00_dtype type = da00_dtype::uint16;
};
template <> struct DataTypeTrait<uint32_t> {
  static constexpr da00_dtype type = da00_dtype::uint32;
};
template <> struct DataTypeTrait<uint64_t> {
  static constexpr da00_dtype type = da00_dtype::uint64;
};
template <> struct DataTypeTrait<float> {
  static constexpr da00_dtype type = da00_dtype::float32;
};
template <> struct DataTypeTrait<double> {
  static constexpr da00_dtype type = da00_dtype::float64;
};
template <> struct DataTypeTrait<char> {
  static constexpr da00_dtype type = da00_dtype::c_string;
};

/// \brief Represents a variable can be stored in DA00 DataArray flatbuffer
/// table.
/// \details The Variable class is used to serialize and deserialize
/// variables using the DA00 schema defined in the FlatBuffers format. The class
/// provides methods for serializing and deserializing variables using the
/// FlatBuffers library.
class Variable {
  std::string Name;
  da00_dtype Type = da00_dtype::none;
  std::vector<uint8_t> Data = {};
  std::vector<std::string> Axes = {};
  std::vector<int64_t> Shape = {};
  std::optional<std::string> Unit;
  std::optional<std::string> Label;
  std::optional<std::string> Source;

public:
  /// \brief Create a new variable with a name.
  /// \param Name The name of the variable
  explicit Variable(std::string Name) : Name(std::move(Name)) {}
  Variable(std::string Name, std::vector<std::string> Axes,
           std::vector<int64_t> Shape)
      : Name(std::move(Name)), Axes(std::move(Axes)), Shape(std::move(Shape)) {
    if (Axes.size() != Shape.size())
      throw std::runtime_error("Input value shapes inconsistent");
  }

  /// \brief copies data vector to the variable.
  /// \param d The data vector to copy to the variable
  template <class T> Variable &data(const std::vector<T> &d) {
    if (da00_dtype::none == DataTypeTrait<T>::type) {
      throw std::runtime_error(
          "Unsupported data type in Variable serialization!");
    }
    // Allow unspecified-shape vector input
    if (Shape.empty())
      Shape = {static_cast<int64_t>(d.size())};
    // Check that the provided shape specify the same number of elements
    if (const auto total =
            std::reduce(Shape.begin(), Shape.end(), 1, std::multiplies());
        total != static_cast<int64_t>(d.size())) {
      throw std::runtime_error("Inconsistent data and shape arrays");
    }
    // Stash-away the da00_dtype equivalent to T
    Type = DataTypeTrait<T>::type;
    // Reinterpret the provided data as a series of unsigned 1 byte integers
    auto Bytes = sizeof(T) * d.size() / sizeof(uint8_t);
    Data.resize(Bytes);
    std::memcpy(Data.data(), d.data(), Bytes);

    return *this;
  }

  /// \brief give back a copy from the data vector of this variable.
  /// \return The data vector of this variable
  template <class T> std::vector<T> data() const {
    const auto count = sizeof(uint8_t) * Data.size() / sizeof(T);
    if (const auto total =
            std::reduce(Shape.begin(), Shape.end(), 1, std::multiplies());
        total != static_cast<int64_t>(count)) {
      std::stringstream ss;
      ss << "Inconsistent data and shape arrays for specified type because "
         << total << " != " << count;
      throw std::runtime_error(ss.str());
    }
    std::vector<T> Out(count);
    std::memcpy(Out.data(), Data.data(), Data.size());
    return Out;
  }

  /// \brief Copy over axis names from a const vector of strings.
  /// \param ax const vector of strings to copy to the variable
  Variable &axes(const std::vector<std::string> &ax) {
    Axes.clear();
    Axes.reserve(ax.size());
    for (const auto &a : ax)
      Axes.push_back(a);
    return *this;
  }

  /// \brief Move axis name vector into the Variable object.
  /// \param ax vector of strings to move to the variable
  /// Input string will be in unspecified state after this call
  Variable &axes(std::vector<std::string> &&ax) {
    Axes = std::move(ax);
    return *this;
  }

  template <class T>
  std::enable_if_t<std::is_integral_v<T>, Variable &> &
  shape(const std::vector<T> &sh) {
    Shape.clear();
    Shape.reserve(sh.size());
    for (const auto &s : sh)
      Shape.push_back(static_cast<int64_t>(s));
    return *this;
  }

  /// \brief Move the a vector of shapes into the object
  /// \param sh The vector of shapes to move to the variable object
  /// Input string will be in unspecified state after this call
  /// \return returns a reference to self
  Variable &shape(std::vector<int64_t> &&sh) {
    Shape = std::move(sh);
    return *this;
  }

  /// \brief Move the unit string into the variable object
  /// \param u The unit string to move to the variable object
  /// Input string will be in unspecified state after this call
  /// \return returns a reference to self
  Variable &unit(std::string &&u) {
    Unit = std::move(u);
    return *this;
  }

  /// \brief Move the label string into the variable object
  /// \param l The label string to move to the variable object
  /// Input string will be in unspecified state after this call
  /// \return returns a reference to self
  Variable &label(std::string &&l) {
    Label = std::move(l);
    return *this;
  }

  /// \brief Move the source string into the variable object
  /// \param s The source string to move to the variable object
  /// Input string will be in unspecified state after this call
  /// \return returns a reference to self
  Variable &source(std::string &&s) {
    Source = std::move(s);
    return *this;
  }

  /// \brief Copy the unit string into the variable object
  /// \param u The unit string to copy to the variable object
  /// \return returns a reference to self
  Variable &unit(const std::string &u) {
    Unit = u;
    return *this;
  }

  /// \brief Copy the label string into the variable object
  /// \param l The label string to copy to the variable object
  /// \return returns a reference to self
  Variable &label(const std::string &l) {
    Label = l;
    return *this;
  }

  /// \brief Copy the source string into the variable object
  /// \param s The source string to copy to the variable object
  /// \return returns a reference to self
  Variable &source(const std::string &s) {
    Source = s;
    return *this;
  }

  /// \brief Get the name of the variable.
  /// \return The name of the variable
  const std::string &getName() const { return Name; }

  /// \brief Get the data vector of the variable.
  /// \return The data vector of the variable
  const std::vector<uint8_t> &getData() const { return Data; }

  /// \brief Get the axes vector of the variable.
  /// \return The axes vector of the variable
  const std::vector<std::string> &getAxes() const { return Axes; }

  /// \brief Get the unit string of the variable.
  /// \return The unit string of the variable
  const std::optional<std::string> &getUnit() const { return Unit; }

  /// \brief Get the label string of the variable.
  /// \return The label string of the variable
  const std::optional<std::string> &getLabel() const { return Label; }

  /// \brief Get the source string of the variable.
  /// \return The source string of the variable
  const std::optional<std::string> &getSource() const { return Source; }

  /// \brief Check if two variables are equal.
  /// \param Other The other variable to compare
  /// \return True if the variables are equal, false otherwise
  bool operator==(const Variable &Other) const {
    if (Name != Other.Name || Data != Other.Data || Axes != Other.Axes ||
        Shape != Other.Shape || Unit != Other.Unit || Label != Other.Label ||
        Source != Other.Source)
      return false;
    return true;
  }

  /// \brief Serialize the variable to a flatbuffer.
  /// \details The variable is serialized to a flatbuffer and stored in the
  /// buffer of the abstract serializer
  /// \param Builder The flatbuffer builder to use for serialization
  /// \return The flatbuffer offset of the serialized variable
  auto pack(flatbuffers::FlatBufferBuilder &Builder) const {
    const auto source =
        Source.has_value() ? Builder.CreateString(Source.value()) : 0;
    const auto label =
        Label.has_value() ? Builder.CreateString(Label.value()) : 0;
    const auto unit = Unit.has_value() ? Builder.CreateString(Unit.value()) : 0;
    auto name = Builder.CreateString(Name);
    const auto axes = Builder.CreateVectorOfStrings(Axes);

    // shape should be int64_t but is size_t, which is (probably) uint64_t:
    std::vector<int64_t> signed_shape;
    signed_shape.reserve(Shape.size());
    std::transform(Shape.begin(), Shape.end(), std::back_inserter(signed_shape),
                   [](const size_t a) { return static_cast<int64_t>(a); });
    const auto shape = Builder.CreateVector<int64_t>(signed_shape);

    const auto data = Builder.CreateVector<uint8_t>(Data);
    return Createda00_Variable(Builder, name, unit, label, source, Type, axes,
                               shape, data);
  }

  /// \brief Deserialize a variable from a flatbuffer.
  /// \details The variable is deserialized from a flatbuffer and stored in the
  /// buffer of the abstract serializer
  /// \param Buffer The flatbuffer to deserialize
  explicit Variable(da00_Variable const *Buffer) : Name{Buffer->name()->str()} {
    if (Buffer->unit())
      Unit = Buffer->unit()->str();
    if (Buffer->label())
      Label = Buffer->label()->str();
    if (Buffer->source())
      Source = Buffer->source()->str();
    if (Buffer->shape()) {
      Shape = std::vector(Buffer->shape()->begin(), Buffer->shape()->end());
    }
    if (Buffer->axes()) {
      std::vector<std::string> axes;
      axes.reserve(Buffer->axes()->size());
      for (const auto &axis : *Buffer->axes()) {
        axes.push_back(axis->str());
      }
      if (!axes.empty())
        Axes = axes;
    }
    if (Buffer->data()) {
      Data = std::vector(Buffer->data()->begin(), Buffer->data()->end());
    }
    Type = Buffer->data_type();
  }
};

/// \brief Represents a data array with a source name, timestamp, and a vector
/// of variables.
/// \details The DataArray class is used to serialize and
/// deserialize data arrays using the DA00 schema defined in the FlatBuffers
/// format. The class provides methods for serializing and deserializing data
/// arrays using the FlatBuffers library.
class DataArray {
  std::string SourceName;
  TimeDurationNano ReferenceTime;
  std::vector<Variable> Data;

public:
  /// \brief Deserialize a data array from a flatbuffer.
  /// \param SourceName The name of the data source
  /// \param ReferenceTime The reference time for this data array
  /// \param Data The vector of variables to include in the data array
  DataArray(std::string SourceName, TimeDurationNano ReferenceTime,
            std::vector<Variable> Data)
      : SourceName(std::move(SourceName)), ReferenceTime(std::move(ReferenceTime)),
        Data(std::move(Data)) {}

  /// \brief Create new data with deserialization from a flatbuffer.
  /// \param Buffer The flatbuffer to deserialize
  explicit DataArray(da00_DataArray const *Buffer)
      : SourceName(Buffer->source_name()->str()),
        ReferenceTime(TimeDurationNano(Buffer->timestamp())) {
    if (Buffer->data()) {
      Data.reserve(Buffer->data()->size());
      for (const auto &variable : *Buffer->data()) {
        Data.emplace_back(variable);
      }
    }
  }

  /// \brief Serialize the data array to a flatbuffer.
  /// \details The data array is serialized to a flatbuffer and stored in the
  /// buffer of the abstract serializer
  /// \param Builder The flatbuffer builder to use for serialization
  /// \return The flatbuffer offset of the serialized data array
  auto pack(flatbuffers::FlatBufferBuilder &Builder) const {
    const auto SourceNameOffset = Builder.CreateString(this->SourceName);
    const auto Time = ReferenceTime.count();

    std::vector<flatbuffers::Offset<da00_Variable>> DA00VariableOffsets;
    DA00VariableOffsets.reserve(Data.size());
    for (const auto &d : Data)
      DA00VariableOffsets.push_back(d.pack(Builder));
    const auto DataOffsets = Builder.CreateVector(DA00VariableOffsets);
    return Createda00_DataArray(Builder, SourceNameOffset, Time, DataOffsets);
  }

  /// \brief Check if two data arrays are equal.
  /// \param Other The other data array to compare
  /// \return True if the data arrays are equal, false otherwise
  bool operator==(const DataArray &Other) const {
    return SourceName == Other.SourceName &&
           ReferenceTime == Other.ReferenceTime && Data == Other.Data;
  }

  /// \brief Get a const reference to the data vector for read only purposes.
  /// \return A const reference to the data vector
  const std::vector<Variable> &getData() const { return Data; }

  /// \brief Get the source name of the data array.
  /// \return The source name of the data array
  const std::string &getSourceName() const { return SourceName; }

  /// \brief Get the timestamp of the data array.
  /// \return The timestamp of the data array
  const TimeDurationNano &getTimeStamp() const { return ReferenceTime; }
};
;

}; // namespace da00flatbuffers