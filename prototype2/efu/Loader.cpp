/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Detector.h>
#include <dlfcn.h>
#include <efu/Loader.h>
#include <iostream>

using namespace std;

Loader::~Loader() { dlclose(handle); }

Loader::Loader(const std::string lib, void * args) {
  char **name;
  const char *libname = ("./" + lib + ".so").c_str();
  DetectorFactory *myFactory;

  if ((handle = dlopen(libname, RTLD_NOW)) == 0) {
    cout << "Could not open library " << libname << " : " << dlerror() << endl;
    return;
  }

  if ((name = (char **)dlsym(handle, "classname")) == 0) {
    cout << "Could not find classname in " << libname << endl;
    return;
  }

  if (!(myFactory = (DetectorFactory *)dlsym(handle, "Factory"))) {
    cout << "Could not find Factory in " << libname << endl;
    return;
  }

  detector = myFactory->create(args);
}
