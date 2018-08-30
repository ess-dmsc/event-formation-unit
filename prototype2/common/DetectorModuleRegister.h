/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Header for a system for registering detector modules when statically
/// linking the modules
/// to the EFU executable.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Detector.h>
#include <string>

/// \brief Stores a factory function and CLI set-up function for a detector module.
struct DetectorModuleSetup {
  DetectorModuleSetup() = default;
  DetectorModuleSetup(std::shared_ptr<DetectorFactoryBase> Factory,
                      std::function<void(CLI::App &)> Setup)
      : DetectorFactory(Factory), CLISetup(Setup) {}
  std::shared_ptr<DetectorFactoryBase> DetectorFactory;
  std::function<void(CLI::App &)> CLISetup;
};

namespace DetectorModuleRegistration {
/// \brief Returns a reference to a static map which stores detector factories.
std::map<std::string, DetectorModuleSetup> &getFactories();

/// \brief Add a new detector module.
/// \throws std::runtime_error If key already exists.
void addDetectorModule(std::string key, DetectorModuleSetup Module);

/// \brief Find a detector module, given a key.
/// \throws std::runtime_error If the key is not found.
DetectorModuleSetup &find(std::string const &DetectorModuleName);


/// \brief Register a detector module by instantiating a member of
/// this class.
///
/// Usage example: DetectorModuleRegistration::Registrar<Sonde> Register("Sonde", CLIPopulatorFunction);
template <class Module> class Registrar {
public:
  /// \brief Constructor used to register a statically linked detector module.
  ///
  /// \param[in] DetectorName The key (name) of the module.
  /// \param[in] CLIPopulator Function pointer (std::function<>) to a function which can populate the CLI interface with additional command line arguments. Can be nullptr.
  explicit Registrar(std::string DetectorName,
                     std::function<void(CLI::App &)> CLIPopulator) {
    std::shared_ptr<DetectorFactoryBase> Factory(new DetectorFactory<Module>());
    addDetectorModule(DetectorName, DetectorModuleSetup(Factory, CLIPopulator));
  };
};
} // namespace DetectorModuleRegistration
