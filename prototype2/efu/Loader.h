/** Copyright (C) 2016 European Spallation Source */

/** @file
 *
 *  @brief Loads detector plugins (shared objects)
 */

#pragma once
#include <memory>
#include <string>

class Detector;

class Loader {
private:
  void *handle{nullptr};

public:
  std::shared_ptr<Detector> detector{nullptr};

  /** @brief Load instrument plugin from detector name
   *  @param name Instrument name - .so suffix will be added
   */
  Loader(std::string name);

  /** @brief minimal destructor */
  ~Loader();
};
