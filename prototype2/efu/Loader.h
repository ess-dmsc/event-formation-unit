/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Loads detector plugins (shared objects)
 */

#pragma once
#include <CLI11.hpp>
#include <common/Detector.h>
#include <functional>
#include <memory>
#include <string>

class Loader {
private:
  void *handle{nullptr};

  DetectorFactoryBase *myFactory{nullptr};

  std::function<void(CLI::App &CLIParser)> ParserPopulator;

public:
  //  std::shared_ptr<Detector> detector{nullptr};

  /** @brief Load instrument plugin from detector name
   *  @param name Instrument name - .so suffix will be added
   */
  Loader();

  Loader(Detector *detector);

  bool loadPlugin(std::string lib);
  void unloadPlugin();

  /** @brief minimal destructor */
  ~Loader();

  bool IsOk();

  std::shared_ptr<Detector> createDetector(BaseSettings settings);

  std::function<void(CLI::App &CLIParser)> GetCLIParserPopulator() {
    return ParserPopulator;
  };
};
