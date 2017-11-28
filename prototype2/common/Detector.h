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

struct StdSettings {
  std::string DetectorAddress;
  std::uint16_t DetectorPort;
  std::int32_t DetectorRxBufferSize;
  std::int32_t DetectorTxBufferSize;
  std::string KafkaBrokerAddress;
  std::uint16_t KafkaBrokerPort;
  std::string KafkaTopic;
};

class Detector {

public:
  Detector(StdSettings settings) : EFUSettings(settings) {};
  // default constructor, all instruments must implement these methods
  /** @brief generic pthread argument
   * @param arg user supplied pointer to pthread argument data
   */
   virtual void input_thread() {
      XTRACE(INIT, ALW, "loaded detector has no input stage\n");
    }

   virtual void processing_thread() {
     XTRACE(INIT, ALW, "loaded detector has no processing stage\n");
   }

   virtual void output_thread() {
     XTRACE(INIT, ALW, "loaded detector has no output stage\n");
   }

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
  
  void stop_threads() {
    runThreads.store(false);
  };

protected:
  std::atomic_bool runThreads{true};
  StdSettings EFUSettings;
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
  virtual std::shared_ptr<Detector> create(StdSettings settings) = 0;
};
