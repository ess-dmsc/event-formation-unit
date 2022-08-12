/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 * \brief Implementation for a system for registering detector modules when
 * statically linking the
 * modules to the EFU executable.
 */

#include "DetectorModuleRegister.h"

namespace DetectorModuleRegistration {
std::map<std::string, DetectorModuleSetup> &getFactories() {
  static std::map<std::string, DetectorModuleSetup> Modules;
  return Modules;
}

DetectorModuleSetup &find(std::string const &DetectorModuleName) {
  auto &Modules = getFactories();
  auto FoundModule = Modules.find(DetectorModuleName);
  if (FoundModule == Modules.end()) {
    throw std::runtime_error("No detector module found with that name.");
  }
  return FoundModule->second;
}

void addDetectorModule(std::string const &DetectorModuleName,
                       DetectorModuleSetup Module) {
  auto &Modules = getFactories();
  if (Modules.find(DetectorModuleName) != Modules.end()) {
    throw std::runtime_error("Detector module is already in list of modules.");
  }
  Modules[DetectorModuleName] = Module;
}
} // namespace DetectorModuleRegistration
