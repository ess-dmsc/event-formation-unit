/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for launching processing threads
 */

#pragma once
#include <efu/Loader.h>
#include <vector>

class EFUArgs;

class Launcher {
public:
  /** @brief Launches previously Loaded detector functions
   *  @param ld Dynamic detector object (from Loader)
   *  @param args Arguments to be passed to threads
   *  @param lcore id for pthread affinity for input_thread
   *  @param lcore id for pthread affinity for processing_thread
   *  @param lcore id for pthread affinity for output_thread
  */
  Launcher(Loader *ld, EFUArgs *args, int input, int processing, int output);

  /** @todo document */
  Launcher(Loader *ld, EFUArgs *args, std::vector<int>& cpus);

private:
  static void input_thread(Loader *load, EFUArgs *args);
  static void processing_thread(Loader *load, EFUArgs *args);
  static void output_thread(Loader *load, EFUArgs *args);

  void launch(int lcore, void (*func)(Loader *, EFUArgs *), Loader *ld,
              EFUArgs *args);
};
