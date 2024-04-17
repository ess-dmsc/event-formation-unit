//
// Created by Gregory Tucker, European Spallation Source ERIC on 2024-03-01
//

#pragma once

#include "DA00Tables.h"
#include "common/memory/span.hpp"
#include "da00_dataarray_generated.h"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace da00_faltbuffers {

/// \brief A class to handle sending 1-D data collected over a frame to Kafka
/// using da00 flatbuffers schema
template <class T, class R = T> class Frame1DHistogramBuilder {
  using data_t = std::vector<T>;
  using time_t = int32_t;

  std::string _topic;
  time_t _period;
  time_t _count;
  std::string _name;
  std::string _unit;
  std::string _time_unit;

public:
  /// \brief Construct a Frame1DHistogramSender
  Frame1DHistogramBuilder(
      std::string topic, //!< Kafka stream topic destination
      const time_t
          period, //!< length of time of one frame in the specified units
      const time_t bin_count, //!< number of time bins used
      std::string name,       //!< name of the binned data
      std::string unit,       //!< unit of the binned data
      std::string time_unit =
          "millisecond" //!< unit of time used for period, and the binned axis
      )
      : _topic(std::move(topic)), _period(period), _count(bin_count),
        _name(std::move(name)), _unit(std::move(unit)),
        _time_unit(std::move(time_unit)) {}

  nonstd::span<const uint8_t> serialize(const data_t &data) const {
    if (static_cast<time_t>(data.size()) != _count) {
      std::stringstream ss;
      ss << "Expected data to serialize to have " << _count
         << " elements but provided " << data.size();
      throw std::runtime_error(ss.str());
    }
    auto x = axis();
    auto y = (Variable(_name, {"t"}, {_count})
                  .unit(_unit)
                  .source("histogram_sender")
                  .label("Frame 1D Histogram intensity")
                  .data(data));
    const auto dataarray = DataArray("histogram_sender", {x, y});
    flatbuffers::FlatBufferBuilder builder(1024);
    builder.Finish(dataarray.pack(builder));

    return nonstd::span<const uint8_t>(builder.GetBufferPointer(),
                                       builder.GetSize());
  }

private:
  auto axis() const {
    std::vector<R> ax;
    ax.reserve(_count + 1);
    auto dt = static_cast<R>(_period) / static_cast<R>(_count);
    ax.push_back(0);
    for (auto i = 0; i < _count; ++i)
      ax.push_back(ax.back() + dt);

    return (Variable("t", {"t"}, {_count + 1})
                .unit(_time_unit)
                .source("histogram_sender")
                .label("Frame 1D Histogram time")
                .data(ax));
  }
};

} // namespace da00_faltbuffers