/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 * @brief Header for a system for registering detector modules when statically linking the modules
 * to the EFU executable.
 */

#pragma once
#include <common/Detector.h>

struct DetectorModuleSetup {
  DetectorModuleSetup() = default;
  DetectorModuleSetup(std::shared_ptr<DetectorFactoryBase> Factory, std::function<void(CLI::App&)> Setup) : DetectorFactory(Factory), CLISetup(Setup) {}
  std::shared_ptr<DetectorFactoryBase> DetectorFactory;
  std::function<void(CLI::App&)> CLISetup;
};

namespace DetectorModuleRegistration {
std::map<std::string, DetectorModuleSetup> &getFactories();
void addDetectorModule(std::string key, DetectorModuleSetup Module);

DetectorModuleSetup &find(std::string const &ModuleName);

template <class Module> class Registrar {
public:
  explicit Registrar(std::string DetectorName, std::function<void(CLI::App&)> CLIPopulator) {
    std::shared_ptr<DetectorFactoryBase> Factory(new DetectorFactory<Module>());
    addDetectorModule(DetectorName, DetectorModuleSetup(Factory, CLIPopulator));
  };
};
} // namespace DetectorModuleRegistration
