// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. see LICENSE file
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
#include <cstdint>
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
  int64_t DataBeforeTimeOffsetDropped{0};
  int64_t DataOverPeriodDropped{0};
  int64_t DataOverPeriodLastBin{0};
};

/// \class HistogramSerializer
/// \brief A class that builds a 1D histogram frame for serialization
///
/// \tparam T is the type of the data to be serialized
/// \tparam R is the type of the data used for the axis. This can be time with
/// double precision
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
  const std::string SignalUnit;
  const uint8_t AggregatedFrames{}; 
  const essmath::VectorAggregationFunc<T> AggregateFunction;
  const R BinOffset;
  const enum BinningStrategy BinningStrategy;
  HistrogramSerializerStats Stats;
  std::vector<size_t> BinSizes;
  flatbuffers::FlatBufferBuilder BufferBuilder{};
  int FrameCounter{0};

  data_t DataBins;
  std::vector<R> XAxisValues;

public:
  // clang-format off
  /// \brief Constructor for the HistogramBuilder class.
  ///
  /// \param Source   Data source
  /// \param Period   TDuration one frame in the specified units
  /// \param BinCount Number of used bins
  /// \param DataUnit Bin unit
  /// \param AggregatedFrames Number of pulses to aggregate before send
  /// \param Callback Producer callback function
  /// \param AggFunc  Function used to aggregate bin data
  /// \param Strategy Enum used to select the binning strategy

  explicit HistogramSerializer(
              std::string Source, 
              time_t Period, 
              time_t BinCount, 
              const std::string &DataUnit,
              uint8_t AggregatedFrames = 1, 
              ProducerCallback Callback = {}, 
              R BinOffset = 0,
              essmath::VectorAggregationFunc<T> AggFunc = essmath::SUM_AGG_FUNC<T>,
              enum BinningStrategy Strategy = BinningStrategy::Drop)
    : AbstractSerializer(Callback, Stats)
    , Source(std::move(Source))
    , Period(std::move(Period))
    , BinCount(std::move(BinCount))
    , SignalUnit(std::move(DataUnit))
    , AggregatedFrames(AggregatedFrames)
    , AggregateFunction(std::move(AggFunc))
    , BinOffset(BinOffset)
    , BinningStrategy(std::move(Strategy))
    , Stats()
    , BufferBuilder(BinCount * (sizeof(T) + sizeof(R)) + 256) {
    // clang-format on
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

    if (static_cast<time_t>(BinOffset) > Period) {
      throw std::invalid_argument(
          fmt ::format("binOffset: {}! cannot be greater than Period {}",
                       BinOffset, Period));
    }
    initAxis();
    DataBins = data_t(BinCount, {0, 0});
    XTRACE(INIT, DEB, "Data Bins vector initialized to %zu", BinCount);
  }

  /// \brief Constructor for the HistogramBuilder class.
  /// \details This constructor is used when binning strategy is provided
  explicit HistogramSerializer(
      std::string Source, time_t Period, time_t BinCount, const std::string &Unit,
      enum BinningStrategy Strategy = BinningStrategy::Drop, uint8_t AggregatedFrames = 1, 
      ProducerCallback Callback = {}, R BinOffset = 0,
      essmath::VectorAggregationFunc<T> AggFunc = essmath::SUM_AGG_FUNC<T>)
      : HistogramSerializer(Source, Period, BinCount, Unit, AggregatedFrames, Callback, 
                            BinOffset, AggFunc, Strategy) {}

  /// \brief Copy constructor for the HistogramBuilder class.
  explicit HistogramSerializer(const HistogramSerializer &other)
      : AbstractSerializer(other), Source(other.Source), Period(other.Period),
        BinCount(other.BinCount), SignalUnit(other.SignalUnit),
        Stats(other.Stats), BinOffset(BinOffset),
        AggregateFunction(other.AggregateFunction),
        BinningStrategy(other.BinningStrategy), BinSizes(other.BinSizes),
        AggregatedFrames(other.AggregatedFrames), DataBins(other.DataBins), 
        XAxisValues(other.XAxisValues), BufferBuilder(other.BufferBuilder) {}

  /// \brief This function finds the bin index for a given time.
  /// \note This function marked virtual for testing purposes.
  /// \param Time is the time for which used to calculate the correct bin index
  /// \param Value is the value to be added to the bin
  virtual inline void addEvent(const R &Time, const T &Value) {
    static_assert(DataTypeTrait<R>::type != da00_dtype::none,
                  "Data type R not supported for serialization!");

    // requires that initXaxis create an ascending sorted vector
    R MinValue = XAxisValues.front();                          // minimum value
    R MaxValue = XAxisValues.back();                           // maximum value
    R Step = (MaxValue - MinValue) / (XAxisValues.size() - 1); // step size

    if (Time < MinValue) {
      XTRACE(DATA, DEB, "Time %zi lower then min value %zi data dropped.", Time,
             MinValue);
      Stats.DataBeforeTimeOffsetDropped++;
      return;
    }
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


  /// \brief Sets the reference time for serialization.
  ///
  /// \param Time The reference time in ns precision
  ///
  /// Set reference time and trigger message serialize.
  /// If message is serialized it will be sent to message broker
  virtual void checkAndSetReferenceTime(const TimeDurationNano &Time) override {
    // Produce already collected data before change reference time
    if (!ReferenceTime.has_value()) {
      ReferenceTime = Time;
      return;
    } else if (Time == ReferenceTime.value()) {
      return;
    }
    
    FrameCounter++;
    if (FrameCounter >= AggregatedFrames) {
      produce();
      FrameCounter = 0;
    }

    Stats.ProduceRefTimeTriggered++;
    // Update reference time
    ReferenceTime = Time;
  }

  // Getter function for the Stats member
  HistrogramSerializerStats &stats() { return Stats; }

private:
  /// \brief Serialize the data to a flatbuffer.
  /// \details The data is serialized to a flatbuffer and stored in the buffer
  /// of the abstract serializer
  void serialize() override {
    if (static_cast<time_t>(DataBins.size()) != BinCount) {
      auto msg = fmt::format(
          "Expected data to serialize to have {} elements but provided {}",
          BinCount, DataBins.size());
      throw std::runtime_error(msg);
    }

    std::vector<T> AggregatedBins;
    AggregatedBins.reserve(DataBins.size());
    bool emptyBins = true;
    for (const auto &Data : DataBins) {
      if (Data.second > 0) {
        emptyBins = false;
      }
      AggregatedBins.push_back(AggregateFunction(Data));
    }
    if (!emptyBins) {
      auto XAxis = (da00flatbuffers::Variable("frame_time", {"frame_time"},
                                              {static_cast<int64_t>(BinCount) + 1})
                        .unit("ns")
                        .data(XAxisValues));

      auto YAxis =
          (da00flatbuffers::Variable("signal", {"frame_time"},
                                    {static_cast<int64_t>(BinCount)})
              .unit(SignalUnit)
              .data(AggregatedBins));

      const auto DataArray = da00flatbuffers::DataArray(
          Source, ReferenceTime.value(), {XAxis, YAxis});

      BufferBuilder.Finish(DataArray.pack(BufferBuilder), da00_DataArrayIdentifier());
      // Create a detached buffer for AbstractSerializer produce method. Detached buffer
      // wrap a pointer and size to internal buffer builder objects.
      Buffer = BufferBuilder.Release();
    } else {
      // Set Buffer value in parent.
      // Create an empty buffer used for AbstractSerializer produce method
      Buffer = flatbuffers::DetachedBuffer();
    }

    DataBins.clear();
    DataBins.resize(BinCount, {0, 0});
  }

  /// \brief Initialize the X-axis values.
  /// \details The X-axis values are initialized based on the period and
  /// number of bins. These values are cannot be negative the algorithm
  /// expects ascending sorted values from the X axis.
  void initAxis() {
    static_assert(DataTypeTrait<R>::type != da00_dtype::none,
                  "Data type is not supported for serialization!");

    XAxisValues.reserve(BinCount + 1);
    auto BinWidth =
        (static_cast<R>(Period) - BinOffset) / static_cast<R>(BinCount);
    if (BinWidth < 0) {
      throw std::runtime_error(fmt::format(
          "dt: {}! Cannot serialize negative time intervals for X axis.",
          BinWidth));
    }

    for (auto i = 0; i <= BinCount; ++i) {
      XAxisValues.push_back(BinOffset + i * BinWidth);
      XTRACE(INIT, DEB, "X Axis unit added, index: %zu, value: %zu", i,
             XAxisValues[i]);
    }

    XTRACE(INIT, DEB, "XAxis initialized with size %zu", XAxisValues.size());
  }
};

} // namespace fbserializer
