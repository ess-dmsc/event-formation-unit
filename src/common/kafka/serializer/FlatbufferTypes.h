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
#include <da00_dataarray_generated.h>
#include <numeric>
#include <optional>
#include <sstream>

namespace da00_flatbuffers {

template <class> struct data_type_trait {
  static constexpr DA00Dtype type = DA00Dtype::none;
};
template <> struct data_type_trait<int8_t> {
  static constexpr DA00Dtype type = DA00Dtype::int8;
};
template <> struct data_type_trait<int16_t> {
  static constexpr DA00Dtype type = DA00Dtype::int16;
};
template <> struct data_type_trait<int32_t> {
  static constexpr DA00Dtype type = DA00Dtype::int32;
};
template <> struct data_type_trait<int64_t> {
  static constexpr DA00Dtype type = DA00Dtype::int64;
};
template <> struct data_type_trait<uint8_t> {
  static constexpr DA00Dtype type = DA00Dtype::uint8;
};
template <> struct data_type_trait<uint16_t> {
  static constexpr DA00Dtype type = DA00Dtype::uint16;
};
template <> struct data_type_trait<uint32_t> {
  static constexpr DA00Dtype type = DA00Dtype::uint32;
};
template <> struct data_type_trait<uint64_t> {
  static constexpr DA00Dtype type = DA00Dtype::uint64;
};
template <> struct data_type_trait<float> {
  static constexpr DA00Dtype type = DA00Dtype::float32;
};
template <> struct data_type_trait<double> {
  static constexpr DA00Dtype type = DA00Dtype::float64;
};
template <> struct data_type_trait<char> {
  static constexpr DA00Dtype type = DA00Dtype::c_string;
};

class Variable {
  std::string _name;
  DA00Dtype _type = DA00Dtype::none;
  std::vector<uint8_t> _data = {};
  std::vector<std::string> _axes = {};
  std::vector<int64_t> _shape = {};
  std::optional<std::string> _unit;
  std::optional<std::string> _label;
  std::optional<std::string> _source;

public:
  explicit Variable(std::string name) : _name(std::move(name)) {}
  Variable(std::string name, std::vector<std::string> axes,
           std::vector<int64_t> shape)
      : _name(std::move(name)), _axes(std::move(axes)),
        _shape(std::move(shape)) {
    if (_axes.size() != _shape.size())
      throw std::runtime_error("Input value shapes inconsistent");
  }
  template <class T> Variable &data(const std::vector<T> &d) {
    if (DA00Dtype::none == data_type_trait<T>::type) {
      throw std::runtime_error(
          "Unsupported data type in Variable serialization!");
    }
    // Allow unspecified-shape vector input
    if (_shape.empty())
      _shape = {static_cast<int64_t>(d.size())};
    // Check that the provided shape specify the same number of elements
    if (const auto total =
            std::reduce(_shape.begin(), _shape.end(), 1, std::multiplies());
        total != static_cast<int64_t>(d.size())) {
      throw std::runtime_error("Inconsistent data and shape arrays");
    }
    // Stash-away the da00_dtype equivalent to T
    _type = data_type_trait<T>::type;
    // Reinterpret the provided data as a series of unsigned 1 byte integers
    auto bytes = sizeof(T) * d.size() / sizeof(uint8_t);
    _data.resize(bytes);
    std::memcpy(_data.data(), d.data(), bytes);

    return *this;
  }
  template <class T> std::vector<T> data() const {
    const auto count = sizeof(uint8_t) * _data.size() / sizeof(T);
    if (const auto total =
            std::reduce(_shape.begin(), _shape.end(), 1, std::multiplies());
        total != static_cast<int64_t>(count)) {
      std::stringstream ss;
      ss << "Inconsistent data and shape arrays for specified type because "
         << total << " != " << count;
      throw std::runtime_error(ss.str());
    }
    std::vector<T> out(count);
    std::memcpy(out.data(), _data.data(), _data.size());
    return out;
  }
  Variable &axes(const std::vector<std::string> &ax) {
    _axes.clear();
    _axes.reserve(ax.size());
    for (const auto &a : ax)
      _axes.push_back(a);
    return *this;
  }
  Variable &axes(std::vector<std::string> &&ax) {
    _axes = std::move(ax);
    return *this;
  }
  template <class T>
  std::enable_if_t<std::is_integral_v<T>, Variable &> &
  shape(const std::vector<T> &sh) {
    _shape.clear();
    _shape.reserve(sh.size());
    for (const auto &s : sh)
      _shape.push_back(static_cast<int64_t>(s));
    return *this;
  }
  Variable &shape(std::vector<int64_t> &&sh) {
    _shape = std::move(sh);
    return *this;
  }
  Variable &unit(std::string &&u) {
    _unit = std::move(u);
    return *this;
  }
  Variable &label(std::string &&l) {
    _label = std::move(l);
    return *this;
  }
  Variable &source(std::string &&s) {
    _source = std::move(s);
    return *this;
  }
  Variable &unit(const std::string &u) {
    _unit = u;
    return *this;
  }
  Variable &label(const std::string &l) {
    _label = l;
    return *this;
  }
  Variable &source(const std::string &s) {
    _source = s;
    return *this;
  }

  bool operator==(const Variable &other) const {
    if (_name != other._name || _data != other._data || _axes != other._axes ||
        _shape != other._shape || _unit != other._unit ||
        _label != other._label || _source != other._source)
      return false;
    return true;
  }

  auto pack(flatbuffers::FlatBufferBuilder &builder) const {
    const auto source =
        _source.has_value() ? builder.CreateString(_source.value()) : 0;
    const auto label =
        _label.has_value() ? builder.CreateString(_label.value()) : 0;
    const auto unit =
        _unit.has_value() ? builder.CreateString(_unit.value()) : 0;
    auto name = builder.CreateString(_name);
    const auto axes = builder.CreateVectorOfStrings(_axes);

    // shape should be int64_t but is size_t, which is (probably) uint64_t:
    std::vector<int64_t> signed_shape;
    signed_shape.reserve(_shape.size());
    std::transform(_shape.begin(), _shape.end(),
                   std::back_inserter(signed_shape),
                   [](const size_t a) { return static_cast<int64_t>(a); });
    const auto shape = builder.CreateVector<int64_t>(signed_shape);

    const auto data = builder.CreateVector<uint8_t>(_data);
    return Createda00_Variable(builder, name, unit, label, source, _type, axes,
                               shape, data);
  }

  explicit Variable(da00_Variable const *buffer)
      : _name{buffer->name()->str()} {
    if (buffer->unit())
      _unit = buffer->unit()->str();
    if (buffer->label())
      _label = buffer->label()->str();
    if (buffer->source())
      _source = buffer->source()->str();
    if (buffer->shape()) {
      _shape = std::vector(buffer->shape()->begin(), buffer->shape()->end());
    }
    if (buffer->axes()) {
      std::vector<std::string> axes;
      axes.reserve(buffer->axes()->size());
      for (const auto &axis : *buffer->axes()) {
        axes.push_back(axis->str());
      }
      if (!axes.empty())
        _axes = axes;
    }
    if (buffer->data()) {
      _data = std::vector(buffer->data()->begin(), buffer->data()->end());
    }
    _type = buffer->data_type();
  }
};

class DataArray {
  using time_t = std::chrono::time_point<std::chrono::system_clock>;
  std::string _source_name;
  time_t _timestamp;
  std::vector<Variable> _data;

public:
  DataArray(std::string source_name, std::vector<Variable> data)
      : _source_name(std::move(source_name)),
        _timestamp(std::chrono::system_clock::now()), _data(std::move(data)) {}

  explicit DataArray(da00_DataArray const *buffer)
      : _source_name(buffer->source_name()->str()),
        _timestamp(time_t(std::chrono::nanoseconds(buffer->timestamp()))) {
    if (buffer->data()) {
      _data.reserve(buffer->data()->size());
      for (const auto &variable : *buffer->data()) {
        _data.emplace_back(variable);
      }
    }
  }

  auto pack(flatbuffers::FlatBufferBuilder &builder) const {
    const auto source_name = builder.CreateString(_source_name);
    const auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          _timestamp.time_since_epoch())
                          .count();

    std::vector<flatbuffers::Offset<da00_Variable>> offsets;
    offsets.reserve(_data.size());
    for (const auto &d : _data)
      offsets.push_back(d.pack(builder));
    const auto data = builder.CreateVector(offsets);
    return Createda00_DataArray(builder, source_name, time, data);
  }

  bool operator==(const DataArray &other) const {
    return _source_name == other._source_name &&
           _timestamp == other._timestamp && _data == other._data;
  }
};

}; // namespace da00_faltbuffers