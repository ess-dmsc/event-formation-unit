/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Factory and Class for dynamically loadable detector types
 */

#pragma once
#include <memory>
#include <stdio.h>
#include <string>

class Detector {

public:
  // default constructor, all instruments must implement these methods
  /** @brief generic pthread argument
   * @param arg user supplied pointer to pthread argument data
   */
  virtual void input_thread() { printf("no input stage\n"); }

  virtual void processing_thread() { printf("no processing stage\n"); }

  virtual void output_thread() { printf("no output stage\n"); }

  /** @brief optional destructor */
  virtual ~Detector() {}

  /** @brief document */
  virtual int statsize() { return 0; }

  /** @brief document */
  virtual int64_t statvalue(size_t __attribute__((unused)) index) {
    return (int64_t)-1;
  }

  /** @brief document */
  virtual std::string &statname(size_t __attribute__((unused)) index) {
    return noname;
  }

  virtual const char *detectorname() { return "no detector"; }

private:
  std::string noname{""};
};

class DetectorFactory {
public:
  /** @brief creates the detector object. All instruments must implement this
  */
  virtual std::shared_ptr<Detector> create(void *opts) = 0;
};
