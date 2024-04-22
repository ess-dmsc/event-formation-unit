// Copyright (C) 2024 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of Frame1DHistogramSerializer, based on the da00
/// schema for serialization
///
/// See https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#pragma once

#include <AbstractSerializer.h>
#include <FlatbufferTypes.h>
#include <common/kafka/Producer.h>
#include <common/math/NumericalMath.h>
#include <common/memory/span.hpp>
#include <flatbuffers/flatbuffers.h>

namespace serializer {

using namespace da00_flatbuffers;
using namespace essmath;

// type trait for type supported as R parameter in the template
template <class> struct data_type_trait {
  static constexpr da00_dtype type = da00_dtype::none;
};
template <> struct data_type_trait<int8_t> {
  static constexpr da00_dtype type = da00_dtype::int8;
};
template <> struct data_type_trait<int16_t> {
  static constexpr da00_dtype type = da00_dtype::int16;
};
template <> struct data_type_trait<int32_t> {
  static constexpr da00_dtype type = da00_dtype::int32;
};
template <> struct data_type_trait<int64_t> {
  static constexpr da00_dtype type = da00_dtype::int64;
};
template <> struct data_type_trait<uint8_t> {
  static constexpr da00_dtype type = da00_dtype::uint8;
};
template <> struct data_type_trait<uint16_t> {
  static constexpr da00_dtype type = da00_dtype::uint16;
};
template <> struct data_type_trait<uint32_t> {
  static constexpr da00_dtype type = da00_dtype::uint32;
};
template <> struct data_type_trait<uint64_t> {
  static constexpr da00_dtype type = da00_dtype::uint64;
};
template <> struct data_type_trait<float> {
  static constexpr da00_dtype type = da00_dtype::float32;
};
template <> struct data_type_trait<double> {
  static constexpr da00_dtype type = da00_dtype::float64;
};

/// @class Frame1DHistogramBuilder
/// @brief A class that builds a 1D histogram frame for serialization.
///
/// \tparam T is the type of the data to be serialized
/// \tparam R is the type of the data used for the axis. This can be time with
/// double precession
///
/// The Frame1DHistogramBuilder class is responsible for building a 1D histogram
/// frame for serialization. It provides methods to add data to the histogram,
/// serialize the data, and initialize the X-axis values.
template <class T, class R = T>
class Frame1DHistogramBuilder : public AbstractSerializer {
  using data_t = std::vector<std::vector<T>>;
  using time_t = int32_t;

  std::string _topic;
  time_t _period;
  time_t _binCount;
  std::string _name;
  std::string _unit;
  std::string _timeUnit;
  std::vector<R> _xAxis;
  data_t _data;
  VectorAggregationFunc<T> aggregateFunction;

public:
  /// \brief Constructor for the Frame1DHistogramBuilder class.
  ///
  /// \param topic is the Kafka stream topic destination
  /// \param period is the length of time of one frame in the specified units
  /// \param binCount is the number of the bins used
  /// \param name is the name of the binned data
  /// \param unit is the unit of the binned data
  /// \param timeUnit is the unit of time used for period, and the binned axis
  /// \param callback is the producer callback function
  /// \param aggFunc is the aggregation function used to aggregate the data
  /// inide the bins
  ///
  Frame1DHistogramBuilder(
      std::string topic, const time_t period, const time_t binCount,
      std::string name, std::string unit,
      std::string timeUnit =
          "millisecond", //!< unit of time used for period, and the binned axis
      const ProducerCallback &callback = {},
      const VectorAggregationFunc<T> &aggFunc = SUM_AGG_FUNC<T>)
      : AbstractSerializer(callback), _topic(std::move(topic)), _period(period),
        _binCount(binCount), _name(std::move(name)), _unit(std::move(unit)),
        _timeUnit(std::move(timeUnit)), aggregateFunction(aggFunc) {

    initAxis();
    _data = data_t(_xAxis.size());
  }

  inline void addData(const R rToA, const T data) {
    static_assert(data_type_trait<R>::type != da00_dtype::none,
                  "Data type R not supported for serialization!");
    // requires that initXaxis create an ascending sorted vector
    R minValue = _xAxis.front();                          // minimum value
    R maxValue = _xAxis.back();                           // maximum value
    R step = (maxValue - minValue) / (_xAxis.size() - 1); // step size

    int binIndex = static_cast<int>((rToA - minValue) / step);

    // Reserve space for the data for optimize on performance
    if (_data[binIndex].empty())
      _data[binIndex].reserve(50);

    _data[binIndex].push_back(data);
  }

private:
  /// \brief Serialize the data to a flatbuffer.
  /// \details The data is serialized to a flatbuffer and stored in the buffer
  /// of the abstract serializer
  void serialize() {
    if (static_cast<time_t>(_data.size()) != _binCount) {
      std::stringstream ss;
      ss << "Expected data to serialize to have " << _binCount
         << " elements but provided " << _data.size();
      throw std::runtime_error(ss.str());
    }

    std::vector<T> aggregatedBins;
    aggregatedBins.reserve(_data.size());
    for (auto data : _data) {
      aggregatedBins.push_back(aggregateFunction(data));
    }

    auto x = (Variable("t", {"t"}, {static_cast<time_t>(_binCount)})
                  .unit(_timeUnit)
                  .source("histogram_sender")
                  .label("Frame 1D Histogram time")
                  .data(_xAxis));

    auto y = (Variable(_name, {"t"}, {static_cast<time_t>(_binCount)})
                  .unit(_unit)
                  .source("histogram_sender")
                  .label("Frame 1D Histogram intensity")
                  .data(aggregatedBins));

    const auto dataarray = DataArray("histogram_sender", {x, y});

    flatbuffers::FlatBufferBuilder builder(_binCount * (sizeof(T) + sizeof(R)) +
                                           256);
    builder.Finish(dataarray.pack(builder));

    _buffer = builder.Release();
    _data.clear();
    _data.reserve(_xAxis.size());
  }

  /// \brief Initialize the X-axis values.
  /// \details The X-axis values are initialized based on the period and number
  /// of bins. These values are cannot be negative the algorithm expects
  /// ascending sorted values from the X axis.
  void initAxis() {
    static_assert(data_type_trait<R>::type != da00_dtype::none,
                  "Data type is not supported for serialization!");
    // Check for negative values in the bin count and period
    // Current concept not support negative values for bins
    if (_binCount < 0) {
      std::stringstream ss;
      ss << "binCount: " << _binCount
         << "! Cannot serialize negative bin counts";
      throw std::domain_error(ss.str());
    }

    if (_period < 0) {
      std::stringstream ss;
      ss << "period: " << _period
         << "! Cannot serialize negative time intervals for X axis.";
      throw std::domain_error(ss.str());
    }

    _xAxis.reserve(_binCount);
    auto dt = static_cast<R>(_period) / static_cast<R>(_binCount);
    if (dt < 0) {
      throw std::runtime_error(
          "Cannot serialize negative time intervals for X axis.");
    }
    _xAxis.push_back(0);
    for (auto i = 1; i < _binCount; ++i)
      _xAxis.push_back(_xAxis.back() + dt);
  }
};

} // namespace serializer
