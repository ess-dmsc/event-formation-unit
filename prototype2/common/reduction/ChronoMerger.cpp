/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file ChronoMerger.h
/// \brief ChronoMerger class implementation
///
//===----------------------------------------------------------------------===//

#include <common/reduction/ChronoMerger.h>
#include <algorithm>

ChronoMerger::ChronoMerger(uint64_t maximum_latency, size_t modules)
    : maximum_latency_(maximum_latency) {
  latest_.resize(modules, 0);
}

void ChronoMerger::insert(size_t module, NeutronEvent event) {
  latest_[module] = std::max(event.time, latest_.at(module));
  queue_.push_back(event);
}

void ChronoMerger::sort() {
  queue_.sort([](const NeutronEvent &e1, const NeutronEvent &e2) {
    return e1.time < e2.time;
  });
}

void ChronoMerger::reset() {
  latest_.assign(latest_.size(), 0);
}

bool ChronoMerger::empty() const {
  return queue_.empty();
}

uint64_t ChronoMerger::earliest() const {
  return queue_.front().time;
}

uint64_t ChronoMerger::horizon() const {
  uint64_t ret = std::numeric_limits<uint64_t>::max();
  for (const auto &l : latest_)
    ret = std::min(ret, l);
  if (ret == std::numeric_limits<uint64_t>::max())
    return 0;
  return ret;
}

NeutronEvent ChronoMerger::pop_earliest() {
  auto ret = queue_.front();
  queue_.pop_front();
  return ret;
}

bool ChronoMerger::ready() const {
  if (empty())
    return false;
  auto h = horizon();
  if (h < maximum_latency_)
    return false;
  return (earliest() < (h - maximum_latency_));
}
