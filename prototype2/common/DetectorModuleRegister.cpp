/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 * @brief Implementation for a system for registering detector modules when
 * statically linking the
 * modules to the EFU executable.
 */

#include "DetectorModuleRegister.h"

namespace DetectorModuleRegistration {
std::map<std::string, DetectorModuleSetup> &getFactories() {
  static std::map<std::string, DetectorModuleSetup> Modules;
  return Modules;
}

DetectorModuleSetup &find(std::string const &DetectorName) {
  auto &Modules = getFactories();
  auto FoundModule = Modules.find(DetectorName);
  if (FoundModule == Modules.end()) {
    throw std::runtime_error("No detector module found with that name.");
  }
  return FoundModule->second;
}

void addDetectorModule(std::string DetectorName, DetectorModuleSetup Module) {
  auto &Modules = getFactories();
  if (Modules.find(DetectorName) != Modules.end()) {
    throw std::runtime_error("Detector module is already in list of modules.");
  }
  Modules[DetectorName] = Module;
}
} // namespace DetectorModuleRegistration
