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
#include <string>

// \todo move this to separate header
struct NeutronEvent {
  uint64_t time;
  uint32_t pixel_id;

  std::string to_string() const;
};

/// \class ChronoMerger ChronoMerger.h
/// \brief

class ChronoMerger {
public:
  explicit ChronoMerger(uint64_t maximum_latency, size_t modules);

  void insert(size_t module, NeutronEvent event);
  void insert(size_t module, std::list<NeutronEvent>& events);
  void sync_up(size_t module1, size_t module2);
  void sort();
  void reset();

  bool empty() const;
  uint64_t earliest() const;
  uint64_t horizon() const;

  bool ready() const;
  NeutronEvent pop_earliest();

  std::string debug(const std::string& prepend, bool verbose) const;

private:
  std::list<NeutronEvent> queue_;
  std::vector<uint64_t> latest_;
  uint64_t maximum_latency_;
};

