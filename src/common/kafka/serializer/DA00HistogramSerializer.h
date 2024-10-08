// Copyright (C) 2024 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of HistogramSerializer, based on the da00
/// schema for serialization
///
/// See \link https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

#pragma once

#include <common/kafka/serializer/AbstractSerializer.h>
#include <common/kafka/serializer/FlatbufferTypes.h>
#include <common/math/NumericalMath.h>
#include <da00_dataarray_generated.h>
#include <fmt/format.h>

namespace fbserializer {

static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");

// type trait for type supported as R parameter in the template
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

/// \brief Enum class to define possible binning strategies
/// \details The binning strategy defines how to handle data that falls outside
/// from the period of the histogram. The two possible strategies are:
/// - Drop: Data that falls outside the period is dropped
/// - LastBin: Data that falls outside the period is put in the last bin
enum class BinningStrategy { Drop, LastBin };

struct HistrogramSerializerStats : public SerializerStats {
  int64_t DataOverPeriodDropped{0};
  int64_t DataOverPeriodLastBin{0};
};

/// \class HistogramSerializer
/// \brief A class that builds a 1D histogram frame for serialization
///
/// \tparam T is the type of the data to be serialized
/// \tparam R is the type of the data used for the axis. This can be time with
/// double precession
///
/// The HistogramSerializer class is responsible for building a 1D histogram for
/// a certain time period for serialization. It provides methods to add data to
/// the histogram, serialize the data, and initialize the time X-axis values.
template <class T, class R = T>
class HistogramSerializer : public AbstractSerializer {
  using data_t = std::vector<std::pair<T, uint32_t>>;
  using time_t = int32_t;

  const std::string Source;
  const time_t Period;
  const time_t BinCount;
  const std::string Name;
  const std::string Unit;
  const std::string TimeUnit;
  const essmath::VectorAggregationFunc<T> AggregateFunction;
  const enum BinningStrategy BinningStrategy;
  HistrogramSerializerStats Stats;

  std::vector<size_t> BinSizes;

  data_t DataBins;
  std::vector<R> XAxisValues;

public:
  /// \brief Constructor for the HistogramBuilder class.
  ///
  /// \param Source is the source of the data
  /// \param Period is the length of time of one frame in the specified units
  /// \param BinCount is the number of the bins used
  /// \param Name is the name of the binned data
  /// \param DataUnit is the unit of the binned data
  /// \param TimeUnit is the unit of time used for period, and the binned axis
  /// \param Callback is the producer callback function
  /// \param AggFunc is the aggregation function used to aggregate the data
  /// inside the bins
  /// \param Strategy is the enum like binning strategy we can select

  HistogramSerializer(
      std::string Source, time_t Period, time_t BinCount, std::string Name,
      std::string DataUnit, std::string TimeUnit,
      ProducerCallback Callback = {},
      essmath::VectorAggregationFunc<T> AggFunc = essmath::SUM_AGG_FUNC<T>,
      enum BinningStrategy Strategy = BinningStrategy::Drop)
      : AbstractSerializer(Callback, Stats), Source(std::move(Source)),
        Period(std::move(Period)), BinCount(std::move(BinCount)),
        Name(std::move(Name)), Unit(std::move(DataUnit)),
        TimeUnit(std::move(TimeUnit)), AggregateFunction(std::move(AggFunc)),
        BinningStrategy(std::move(Strategy)), Stats() {

    initAxis();
    DataBins = data_t(XAxisValues.size(), {0, 0});
    XTRACE(INIT, DEB, "Data Bins vector initialized to %zu", DataBins.size());
  }

  /// \brief Constructor for the HistogramBuilder class.
  /// \details This constructor is used when binnig strategy is provided
  HistogramSerializer(
      std::string Source, time_t Period, time_t BinCount, std::string Name,
      std::string Unit, std::string TimeUnit, enum BinningStrategy Strategy,
      ProducerCallback Callback = {},
      essmath::VectorAggregationFunc<T> AggFunc = essmath::SUM_AGG_FUNC<T>)
      : HistogramSerializer(Source, Period, BinCount, Name, Unit, TimeUnit,
                            Callback, AggFunc, Strategy) {}

  /// \brief Copy constructor for the HistogramBuilder class.
  HistogramSerializer(const HistogramSerializer &other)
      : AbstractSerializer(other), Source(other.Source), Period(other.Period),
        BinCount(other.BinCount), Name(other.Name), Unit(other.Unit),
        TimeUnit(other.TimeUnit), Stats(other.Stats),
        AggregateFunction(other.AggregateFunction),
        BinningStrategy(other.BinningStrategy), BinSizes(other.BinSizes),
        DataBins(other.DataBins), XAxisValues(other.XAxisValues) {}

  /// \brief This function finds the bin index for a given time.
  /// \note This function marked virtual for testing purposes.
  /// \param Time is the time for which used to calculate the correct bin index
  /// \param Value is the value to be added to the bin
  virtual inline void addEvent(const R Time, const T Value) {
    static_assert(DataTypeTrait<R>::type != da00_dtype::none,
                  "Data type R not supported for serialization!");

    // requires that initXaxis create an ascending sorted vector
    R MinValue = XAxisValues.front();                          // minimum value
    R MaxValue = XAxisValues.back();                           // maximum value
    R Step = (MaxValue - MinValue) / (XAxisValues.size() - 1); // step size

    size_t BinIndex = static_cast<size_t>((Time - MinValue) / Step);
    XTRACE(DATA, DEB, "BinIndex %zu calculated from time: %zi.", BinIndex,
           Time);

    // Check if the binIndex is out of bounds and put edge values in the last
    // bin
    if (BinIndex >= DataBins.size()) {
      if (BinningStrategy == BinningStrategy::Drop) {
        XTRACE(INIT, DEB,
               "The data index %zu higher then binsize data dropped.",
               BinIndex);
        Stats.DataOverPeriodDropped++;
        return;
      }
      BinIndex = DataBins.size() - 1;
      XTRACE(DATA, DEB,
             "The data index %zu higher then binsize data added to last bin.",
             BinIndex);
      Stats.DataOverPeriodLastBin++;
    }

    // If the bin is not empty, get the last value out
    auto &firstPair = DataBins[BinIndex];
    firstPair.first += Value;
    firstPair.second++;
    XTRACE(DATA, DEB,
           "Value %zu added to DataBin[%zu] New value sum: %zu, count: %zu.",
           Value, BinIndex, firstPair.first, firstPair.second);
  }
  // Getter function for the Stats member
  HistrogramSerializerStats &stats() { return Stats; }

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

    const auto DataArray = da00flatbuffers::DataArray(
        Source, ReferenceTime.value(), {XAxis, YAxis});

    flatbuffers::FlatBufferBuilder Builder(BinCount * (sizeof(T) + sizeof(R)) +
                                           256);

    Builder.Finish(DataArray.pack(Builder), da00_DataArrayIdentifier());
    Buffer = Builder.Release();

    DataBins.clear();
    DataBins.resize(XAxisValues.size(), {0, 0});
  }

  /// \brief Initialize the X-axis values.
  /// \details The X-axis values are initialized based on the period and
  /// number of bins. These values are cannot be negative the algorithm
  /// expects ascending sorted values from the X axis.
  void initAxis() {
    static_assert(DataTypeTrait<R>::type != da00_dtype::none,
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

    for (auto i = 1; i < BinCount; ++i) {
      XAxisValues.push_back(XAxisValues.back() + BinWidth);
      XTRACE(INIT, DEB, "X Axis unit added, index: %zu, value: %zu", i,
             XAxisValues[i]);
    }

    XTRACE(INIT, DEB, "XAxis initialized with size %zu", XAxisValues.size());
  }
};

} // namespace fbserializer
