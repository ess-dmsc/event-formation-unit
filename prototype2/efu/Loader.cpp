/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Detector.h>
#include <dlfcn.h>
#include <efu/Loader.h>
#include <iostream>
#include <string.h>
#include <string>

using namespace std;

Loader::~Loader() { dlclose(handle); }

Loader::Loader(const std::string lib, void *args) {

  std::string libname = "./" + lib + ".so";
  const char *libstr = strdup(libname.c_str());

  if ((handle = dlopen(libstr, RTLD_NOW)) == 0) {
    cout << "Could not open library " << libname << " : " << dlerror() << endl;
    free((void*)libstr);
    return;
  }
  free((void*)libstr);

  char **name;
  if ((name = (char **)dlsym(handle, "classname")) == 0) {
    cout << "Could not find classname in " << libname << endl;
    return;
  }

  DetectorFactory *myFactory;
  if (!(myFactory = (DetectorFactory *)dlsym(handle, "Factory"))) {
    cout << "Could not find Factory in " << libname << endl;
    return;
  }

  detector = myFactory->create(args);
}
