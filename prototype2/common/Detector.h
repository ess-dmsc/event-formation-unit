/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Factory and Class for dynamically loadable detector types
 */

#pragma once
#include <common/Trace.h>
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
  std::int32_t DetectorRxBufferSize;
  std::int32_t DetectorTxBufferSize;
  std::string KafkaBrokerAddress;
  std::uint16_t KafkaBrokerPort;
  std::string KafkaTopic;
};

struct ThreadInfo {
  std::function<void(void)> func;
  std::string name;
  std::thread thread;
};

class Detector {
public:
  using ThreadList = std::vector<ThreadInfo>;
  Detector(BaseSettings settings) : EFUSettings(settings) {};
  // default constructor, all instruments must implement these methods
  /** @brief generic pthread argument
   * @param arg user supplied pointer to pthread argument data
   */

  /** @brief optional destructor */
  virtual ~Detector() { printf("Virtual detector destructor called\n");}

  /** @brief document */
  virtual int statsize() { return 0; }

  /** @brief document */
  virtual int64_t statvalue(size_t __attribute__((unused)) index) {
    return (int64_t)-1;
  }

  /** @brief document */
  virtual std::string &statname(size_t __attribute__((unused)) index) {
    return noname;
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
