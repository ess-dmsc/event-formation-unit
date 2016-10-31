/** Copyright (C) 2016 European Spallation Source */

/** @file Loader.h
 *  @brief Loads detector plugins (shared objects)
 */

#pragma once
#include <string>

class Detector;

class Loader {
private:
  void *handle{NULL};

public:
  Detector *detector{NULL};

  ~Loader();
  Loader(std::string lib);
};
