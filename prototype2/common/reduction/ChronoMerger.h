/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file ChronoMerger.h
/// \brief ChronoMerger class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <list>

struct NeutronEvent {
  uint64_t time;
  uint32_t pixel_id;
};

/// \class ChronoMerger ChronoMerger.h
/// \brief

class ChronoMerger {
public:
  explicit ChronoMerger(uint64_t maximum_latency, size_t modules);

  void insert(size_t module, NeutronEvent event);
  void sort();
  void reset();

  bool empty() const;
  uint64_t earliest() const;
  uint64_t horizon() const;

  bool ready() const;
  NeutronEvent pop_earliest();

private:
  std::list<NeutronEvent> queue_;
  std::vector<uint64_t> latest_;
  uint64_t maximum_latency_;
};

