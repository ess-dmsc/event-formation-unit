/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/monitor/Histogram.h>
#include <common/monitor/HistogramSerializer.h>
#include <common/monitor/HitSerializer.h>

// \todo document functions

struct Monitor {
  Monitor() = default;
  Monitor(const std::string &broker, const std::string &topic_prefix,
          const std::string &source_name);
  ~Monitor() { close(); }

  std::shared_ptr<Hists> histograms;
  std::shared_ptr<HitSerializer> hit_serializer;

  void init_histograms(size_t max_range);

  void init_hits(size_t max_readouts);

  void close();

  void produce_now();

private:
  std::string source_name_;
  std::shared_ptr<Producer> producer;
  std::shared_ptr<HistogramSerializer> hist_serializer;
};
