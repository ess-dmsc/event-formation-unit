/** Copyright (C) 2016 European Spallation Source */

#include <efu/Loader.h>

class EFUArgs;

class Launcher {
public:
  Launcher(Loader *ld, EFUArgs *args, int input, int processing, int output);

private:
  static void input_thread(Loader *load, EFUArgs *args);
  static void processing_thread(Loader *load, EFUArgs *args);
  static void output_thread(Loader *load, EFUArgs *args);

  void launch(int lcore, void (*func)(Loader *, EFUArgs *), Loader *ld,
              EFUArgs *args);
};
