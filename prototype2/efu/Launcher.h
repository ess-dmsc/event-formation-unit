/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

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
   *  @param cpus vector of three cpuids for launching input, processing and
   *  output threads.
   */
  Launcher(Loader *ld, std::vector<int> &cpus);

private:
  static void input_thread(Loader *load, EFUArgs *args);
  static void processing_thread(Loader *load, EFUArgs *args);
  static void output_thread(Loader *load, EFUArgs *args);

  void launch(int lcore, void (*func)(Loader *, EFUArgs *), Loader *ld,
              EFUArgs *args);
};
