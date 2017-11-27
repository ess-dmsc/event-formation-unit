/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Loads detector plugins (shared objects)
 */

#pragma once
#include <common/Detector.h>
#include <memory>
#include <string>

class Loader {
private:
  void *handle{nullptr};
  
  DetectorFactory *myFactory{nullptr};
  
public:
//  std::shared_ptr<Detector> detector{nullptr};

  /** @brief Load instrument plugin from detector name
   *  @param name Instrument name - .so suffix will be added
   */
  Loader(std::string name);

  Loader(Detector *detector);

  /** @brief minimal destructor */
  ~Loader();
  
  bool IsOk() {
    if (nullptr == myFactory) {
      return false;
    }
    return true;
  }
  
  std::shared_ptr<Detector> createDetector();
};
