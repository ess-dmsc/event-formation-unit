/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Factory and Class for dynamically loadable detector types
 */

#pragma once
#include <common/Trace.h>
#include <common/NewStats.h>
#include <memory>
#include <stdio.h>
#include <string>
#include <functional>
#include <CLI/CLI11.hpp>
#include <atomic>
#include <thread>

struct BaseSettings {
  std::string DetectorAddress;
  std::uint16_t DetectorPort;
  std::uint16_t CommandServerPort;
  std::int32_t DetectorRxBufferSize;
  std::int32_t DetectorTxBufferSize;
  std::string KafkaBrokerAddress;
  std::uint16_t KafkaBrokerPort;
  std::string GraphiteAddress;
  std::uint16_t GraphitePort;
  std::string KafkaTopic;
  std::string ConfigFile;
  std::uint64_t UpdateIntervalSec;
  std::uint32_t StopAfterSec;
};

struct ThreadInfo {
  std::function<void(void)> func;
  std::string name;
  std::thread thread;
};

class Detector {
public:
  using ThreadList = std::vector<ThreadInfo>;
  Detector(BaseSettings settings) : EFUSettings(settings), StatsTracker("") {};
  // default constructor, all instruments must implement these methods
  /** @brief generic pthread argument
   * @param arg user supplied pointer to pthread argument data
   */

  /** @brief document */
  virtual int statsize() { return StatsTracker.size(); }

  /** @brief document */
  virtual int64_t statvalue(size_t index) {
    return StatsTracker.value(index);
  }

  /** @brief document */
  virtual std::string &statname(size_t index) {
    return StatsTracker.name(index);
  }

  void setStatsPrefix(std::string NewStatsPrefix) {
    StatsTracker.setPrefix(NewStatsPrefix);
  }

  virtual const char *detectorname() { return "no detector"; }

  virtual ThreadList& GetThreadInfo() {
    return Threads;
  };

  virtual void stopThreads() {
    runThreads.store(false);
    for (auto &tInfo : Threads) {
      if (tInfo.thread.joinable()) {
        tInfo.thread.join();
      }
    }
  };

protected:
  void AddThreadFunction(std::function<void(void)> &func, std::string funcName) {
    Threads.emplace_back(ThreadInfo{func, std::move(funcName), std::thread()});
  };
  ThreadList Threads;
  std::atomic_bool runThreads{true};
  BaseSettings EFUSettings;
private:
  NewStats StatsTracker;
  std::string noname{""};
};

struct PopulateCLIParser {
  std::function<void(CLI::App&)> Function;
};

class DetectorFactory {
public:
  /** @brief creates the detector object. All instruments must implement this
  */
  virtual std::shared_ptr<Detector> create(BaseSettings settings) = 0;
};
