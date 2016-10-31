/** Copyright (C) 2016 European Spallation Source */

/** @file Detector.h
 *  @brief Factory and Class for dynamically loadable detector types
 */

#pragma once
#include <stdio.h>

class Detector {

public:
  // default constructor

  virtual void input_thread(void *arg __attribute__((unused))) {
    printf("no input stage\n");
  };

  virtual void processing_thread(void *arg __attribute__((unused))) {
    printf("no processing stage\n");
  };

  virtual void output_thread(void *arg __attribute__((unused))) {
    printf("no output stage\n");
  };

  virtual ~Detector(){};
};

class DetectorFactory {
public:
  virtual Detector *create() = 0;
};
