/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <dlfcn.h>
#include <efu/Loader.h>
#include <iostream>
#include <string.h>
#include <string>

Loader::~Loader() {
  XTRACE(INIT, ALW, "Loader destructor called\n");
  //Remove pointer before closing the handle to prevent accessing freed memory
  ParserPopulator = nullptr;
  dlclose(handle);
}

Loader::Loader(const std::string lib) {

  std::string libname = "./" + lib + ".so";
  const char *libstr = strdup(libname.c_str());

  if ((handle = dlopen(libstr, RTLD_NOW)) == 0) {
    XTRACE(INIT, CRI, "Could not open library %s: %s\n", libname.c_str(), dlerror());
    free((void *)libstr);
    return;
  }
  free((void *)libstr);

  char **name;
  if ((name = (char **)dlsym(handle, "classname")) == 0) {
    XTRACE(INIT, CRI, "Could not find classname in %s\n", libname.c_str());
    return;
  }

  if (!(myFactory = (DetectorFactory *)dlsym(handle, "Factory"))) {
    XTRACE(INIT, CRI, "Could not find Factory in %s\n", libname.c_str());
    return;
  }
  
  PopulateCLIParser *tempParserPopulator = (PopulateCLIParser*)dlsym(handle, "PopulateParser");
  if (nullptr == tempParserPopulator) {
    XTRACE(INIT, WAR, "Unable to find function to populate CLI parser in %s\n", libname.c_str());
  } else {
    if (nullptr == tempParserPopulator->Function) {
      XTRACE(INIT, WAR, "Function to populate CLI parser not set");
    } else {
      ParserPopulator = tempParserPopulator->Function;
    }
  }
}

std::shared_ptr<Detector> Loader::createDetector(BaseSettings settings) {
  return myFactory->create(settings);
}
