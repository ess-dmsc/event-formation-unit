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

#include "FlatbufferTypes.h"
#include "common/kafka/serializer/AbstractSerializer.h"
#include "common/math/NumericalMath.h"
#include "flatbuffers/base.h"
#include "fmt/format.h"

namespace fbserializer {

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

// type trait for type supported as R parameter in the template
template <class> struct DataTypeTrait {
  static constexpr DA00Dtype type = DA00Dtype::none;
};
template <> struct DataTypeTrait<int8_t> {
  static constexpr DA00Dtype type = DA00Dtype::int8;
};
template <> struct DataTypeTrait<int16_t> {
  static constexpr DA00Dtype type = DA00Dtype::int16;
};
template <> struct DataTypeTrait<int32_t> {
  static constexpr DA00Dtype type = DA00Dtype::int32;
};
template <> struct DataTypeTrait<int64_t> {
  static constexpr DA00Dtype type = DA00Dtype::int64;
};
template <> struct DataTypeTrait<uint8_t> {
  static constexpr DA00Dtype type = DA00Dtype::uint8;
};
template <> struct DataTypeTrait<uint16_t> {
  static constexpr DA00Dtype type = DA00Dtype::uint16;
};
template <> struct DataTypeTrait<uint32_t> {
  static constexpr DA00Dtype type = DA00Dtype::uint32;
};
template <> struct DataTypeTrait<uint64_t> {
  static constexpr DA00Dtype type = DA00Dtype::uint64;
};
template <> struct DataTypeTrait<float> {
  static constexpr DA00Dtype type = DA00Dtype::float32;
};
template <> struct DataTypeTrait<double> {
  static constexpr DA00Dtype type = DA00Dtype::float64;
};

enum class BinningStrategy { Drop, LastBin };

struct HistrogramSerializerStats : public SerializerStats {
  int64_t DataOverPeriodDropped{0};
  int64_t DataOverPeriodLastBin{0};
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
class HistogramSerializer : public AbstractSerializer {
  using data_t = std::vector<std::vector<T>>;
  using time_t = int32_t;

  const std::string Topic;
  const std::string Source;
  const time_t Period;
  const time_t BinCount;
  const std::string Name;
  const std::string Unit;
  const std::string TimeUnit;
  HistrogramSerializerStats &Stats;
  const essmath::VectorAggregationFunc<T> AggregateFunction;
  const enum BinningStrategy BinningStrategy;

protected:
  data_t DataBins;
  std::vector<R> XAxisValues;

public:
  /// \brief Constructor for the Frame1DHistogramBuilder class.
  ///
  /// \param Topic is the Kafka stream topic destination
  /// \param Source is the source of the data
  /// \param Period is the length of time of one frame in the specified units
  /// \param BinCount is the number of the bins used
  /// \param Name is the name of the binned data
  /// \param Unit is the unit of the binned data
  /// \param TimeUnit is the unit of time used for period, and the binned axis
  /// \param Stats is the statistics object for tracking serialization
  /// \param Callback is the producer callback function
  /// \param AggFunc is the aggregation function used to aggregate the data
  /// inside the bins
  /// \param Strategy is the enum like binning strategy we can select

  HistogramSerializer(
      const std::string &Topic, const std::string &Source, const time_t &Period,
      const time_t &BinCount, const std::string &Name, const std::string &Unit,
      const std::string TimeUnit, HistrogramSerializerStats &Stats,
      const ProducerCallback Callback = {},
      const essmath::VectorAggregationFunc<T> AggFunc =
          essmath::SUM_AGG_FUNC<T>,
      const enum BinningStrategy Strategy = BinningStrategy::Drop)
      : AbstractSerializer(Callback, Stats), Topic(Topic), Source(Source),
        Period(Period), BinCount(BinCount), Name(Name), Unit(Unit),
        TimeUnit(TimeUnit), Stats(Stats), AggregateFunction(AggFunc),
        BinningStrategy(Strategy) {

    initAxis();
    DataBins = data_t(XAxisValues.size());
  }

  /// \brief This function finds the bin index for a given time.
  /// \param Time is the time for which used to calculate the correct bin index
  /// \param Value is the value to be added to the bin
  inline void addData(const R Time, const T Value) {
    static_assert(DataTypeTrait<R>::type != DA00Dtype::none,
                  "Data type R not supported for serialization!");

    if (static_cast<time_t>(Time) > Period &&
        BinningStrategy == BinningStrategy::Drop) {
      Stats.DataOverPeriodDropped++;
      return;
    } else {
      Stats.DataOverPeriodLastBin++;
    }

    // requires that initXaxis create an ascending sorted vector
    R MinValue = XAxisValues.front();                          // minimum value
    R MaxValue = XAxisValues.back();                           // maximum value
    R Step = (MaxValue - MinValue) / (XAxisValues.size() - 1); // step size

    size_t BinIndex = static_cast<size_t>((Time - MinValue) / Step);

    // Check if the binIndex is out of bounds and put edge values in the last
    // bin
    if (BinIndex >= DataBins.size()) {
      BinIndex = DataBins.size() - 1;
    }

    if (DataBins[BinIndex].empty())
      // Reserve space for the data for optimize on performance
      DataBins[BinIndex].reserve(50);

    DataBins[BinIndex].push_back(Value);
  }

private:
  /// \brief Serialize the data to a flatbuffer.
  /// \details The data is serialized to a flatbuffer and stored in the buffer
  /// of the abstract serializer
  void serialize() {
    if (static_cast<time_t>(DataBins.size()) != BinCount) {
      auto msg = fmt::format(
          "Expected data to serialize to have {} elements but provided {}",
          BinCount, DataBins.size());
      throw std::runtime_error(msg);
    }

    std::vector<T> AggregatedBins;
    AggregatedBins.reserve(DataBins.size());
    for (auto Data : DataBins) {
      AggregatedBins.push_back(AggregateFunction(Data));
    }

    auto XAxis = (da00flatbuffers::Variable("time", {"t"},
                                            {static_cast<time_t>(BinCount)})
                      .unit(TimeUnit)
                      .data(XAxisValues));

    auto YAxis = (da00flatbuffers::Variable(Name, {"a.u."},
                                            {static_cast<time_t>(BinCount)})
                      .unit(Unit)
                      .data(AggregatedBins));

    const auto dataarray =
        da00flatbuffers::DataArray("histogram_sender", {XAxis, YAxis});

    flatbuffers::FlatBufferBuilder Builder(BinCount * (sizeof(T) + sizeof(R)) +
                                           256);
    Builder.Finish(dataarray.pack(Builder));

    Buffer = Builder.Release();
    DataBins.clear();
    DataBins.reserve(XAxisValues.size());
  }

  /// \brief Initialize the X-axis values.
  /// \details The X-axis values are initialized based on the period and number
  /// of bins. These values are cannot be negative the algorithm expects
  /// ascending sorted values from the X axis.
  void initAxis() {
    static_assert(DataTypeTrait<R>::type != DA00Dtype::none,
                  "Data type is not supported for serialization!");
    // Check for negative values in the bin count and period
    // Current concept not support negative values for bins
    if (BinCount < 0) {
      throw std::domain_error(fmt::format(
          "binCount: {}! Cannot serialize negative bin counts", BinCount));
    }

    if (Period < 0) {
      throw std::domain_error(fmt::format(
          "period: {}! Cannot serialize negative time intervals for X axis.",
          Period));
    }

    XAxisValues.reserve(BinCount);
    auto BinWidth = static_cast<R>(Period) / static_cast<R>(BinCount);
    if (BinWidth < 0) {
      throw std::runtime_error(fmt::format(
          "dt: {}! Cannot serialize negative time intervals for X axis.",
          BinWidth));
    }

    XAxisValues.push_back(0);
    for (auto i = 1; i < BinCount; ++i)
      XAxisValues.push_back(XAxisValues.back() + BinWidth);
  }
};

} // namespace fbserializer
