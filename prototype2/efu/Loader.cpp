/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Detector.h>
#include <common/Trace.h>
#include <dlfcn.h>
#include <efu/Loader.h>
#include <iostream>
#include <string.h>
#include <string>

Loader::~Loader() { dlclose(handle); }

Loader::Loader(const std::string lib, void *args) {

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

  DetectorFactory *myFactory;
  if (!(myFactory = (DetectorFactory *)dlsym(handle, "Factory"))) {
    XTRACE(INIT, CRI, "Could not find Factory in %s\n", libname.c_str());
    return;
  }

  detector = myFactory->create(args);
}
