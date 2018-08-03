/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/DetectorModuleRegister.h>
#include <common/Log.h>
#include <dlfcn.h>
#include <efu/Loader.h>
#include <iostream>
#include <string.h>
#include <string>

Loader::~Loader() {
  LOG(Sev::Debug, "Loader destructor called");
  // Remove pointer before closing the handle to prevent accessing freed memory
  unloadPlugin();
}

Loader::Loader() {}

void Loader::unloadPlugin() {
  ParserPopulator = nullptr;
  if (nullptr != handle) {
    dlclose(handle);
  }
}

bool Loader::loadPlugin(const std::string lib) {
  try {
    auto &FoundModule = DetectorModuleRegistration::find(lib);
    ParserPopulator = FoundModule.CLISetup;
    myFactory = FoundModule.DetectorFactory.get();
    LOG(Sev::Info, "Loaded statically linked detector module.");
    return true;
  } catch (std::runtime_error &Error) {
    LOG(Sev::Notice, "Unable to find statically linked detector module with "
        "name\"{}\". Attempting to open external plugin.",
           lib);
  }
  std::vector<std::string> PossibleSuffixes{"", ".so", ".dll", ".dylib"};

  for (auto &CSuffix : PossibleSuffixes) {
    std::string TestLibName = "./" + lib + CSuffix;
    handle = dlopen(TestLibName.c_str(), RTLD_NOW);
    if (handle != nullptr) {
      LOG(Sev::Info, "Loaded library \"{}\".",
            TestLibName);
      break;
    }
    else {
      LOG(Sev::Warning, "Could not open library {}: {}", TestLibName,
          dlerror());
    }
  }
  if (handle == nullptr) {
    LOG(Sev::Error, "All variations of DL failed.");
    return false;
  }

  if (!(myFactory = (DetectorFactoryBase *)dlsym(handle, "Factory"))) {
    LOG(Sev::Error, "Could not find Factory in {}", lib);
    return false;
  }

  PopulateCLIParser *tempParserPopulator =
      (PopulateCLIParser *)dlsym(handle, "PopulateParser");
  if (nullptr == tempParserPopulator) {
    LOG(Sev::Warning, "Unable to find function to populate CLI parser in {}",
           lib);
  } else {
    if (nullptr == tempParserPopulator->Function) {
      LOG(Sev::Warning, "Function to populate CLI parser not set");
    } else {
      ParserPopulator = tempParserPopulator->Function;
    }
  }
  return true;
}

bool Loader::IsOk() {
  if (nullptr == myFactory) {
    return false;
  }
  return true;
}

std::shared_ptr<Detector> Loader::createDetector(BaseSettings settings) {
  return myFactory->create(settings);
}
