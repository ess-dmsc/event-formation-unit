/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Factory and Class for dynamically loadable detector types
 */

#pragma once
#include <common/Trace.h>
#include <memory>
#include <stdio.h>
#include <string>

class Detector {

public:
  // default constructor, all instruments must implement these methods
  /** @brief generic pthread argument
   * @param arg user supplied pointer to pthread argument data
   */
   virtual void input_thread() {
      XTRACE(INIT, ALW, "loaded detector has no input stage\n");
    }

   virtual void processing_thread() {
     XTRACE(INIT, ALW, "loaded detector has no processing stage\n");
   }

   virtual void output_thread() {
     XTRACE(INIT, ALW, "loaded detector has no output stage\n");
   }

  /** @brief optional destructor */
  virtual ~Detector() { printf("Virtual detector destructor called\n");}

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
