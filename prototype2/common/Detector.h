/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Factory and Class for dynamically loadable detector types
 */

#pragma once
#include <memory>
#include <stdio.h>

class Detector {

public:
  // default constructor, all instruments must implement these methods
  /** @brief generic pthread argument
   * @param arg user supplied pointer to pthread argument data
   */
  virtual void input_thread(void *arg __attribute__((unused))) {
    printf("no input stage\n");
  };

  virtual void processing_thread(void *arg __attribute__((unused))) {
    printf("no processing stage\n");
  };

  virtual void output_thread(void *arg __attribute__((unused))) {
    printf("no output stage\n");
  };

  /** @brief optional destructor */
  virtual ~Detector(){};
};

class DetectorFactory {
public:
  /** @brief creates the detector object. All instruments must implement this
  */
  virtual std::shared_ptr<Detector> create() = 0;
};
