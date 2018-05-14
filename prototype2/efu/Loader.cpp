/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <dlfcn.h>
#include <efu/Loader.h>
#include <iostream>
#include <string.h>
#include <string>
#include <common/DetectorModuleRegister.h>


Loader::~Loader() {
  XTRACE(INIT, ALW, "Loader destructor called\n");
  // Remove pointer before closing the handle to prevent accessing freed memory
  unloadPlugin();
}

Loader::Loader() {
  
}

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
    XTRACE(INIT, INF, "Loaded statically linked detector module.");
    return true;
  } catch (std::runtime_error &Error) {
    XTRACE(INIT, INF, "Unable to find statically linked detector module with name\"%s\". Attempting to open external plugin.", lib.c_str());
  }
  std::string libname = "./" + lib + ".so";
  const char *libstr = strdup(libname.c_str());

  if ((handle = dlopen(libstr, RTLD_NOW)) == 0) {
    XTRACE(INIT, CRI, "Could not open library %s: %s\n", libname.c_str(),
           dlerror());
    free((void *)libstr);
    return false;
  }
  free((void *)libstr);

  if (!(myFactory = (DetectorFactoryBase *)dlsym(handle, "Factory"))) {
    XTRACE(INIT, CRI, "Could not find Factory in %s\n", libname.c_str());
    return false;
  }

  PopulateCLIParser *tempParserPopulator =
      (PopulateCLIParser *)dlsym(handle, "PopulateParser");
  if (nullptr == tempParserPopulator) {
    XTRACE(INIT, WAR, "Unable to find function to populate CLI parser in %s\n",
           libname.c_str());
  } else {
    if (nullptr == tempParserPopulator->Function) {
      XTRACE(INIT, WAR, "Function to populate CLI parser not set");
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
