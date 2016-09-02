#include <Pipeline.h>
#include <iostream>
#include <dlfcn.h>

Pipeline::Pipeline(const char * name)
{
  std::cout << "Loading instrument pipeline " << name << "\n";;
  mHandle = dlopen(name, RTLD_NOW);

  if (mHandle == NULL) {
     std::cout << "unable to load library " << name << "\n";
     return;
  }
  mRx = (int (*)(int))dlsym(mHandle, "receive");
}

int Pipeline::Rx(int a)
{
   return mRx(a);
}
