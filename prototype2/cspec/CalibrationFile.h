/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Load CSPEC wire and grid calibratyion from file
 */

#pragma once

#include <string>

class CalibrationFile {
public:

  /** @todo document    */
  int load(std::string calibration, char * wirecal, char * gridcal);

  /** @todo document */
  int save(std::string calibration, char * wirecal, char * gridcal);

private:

  /** @todo document    */
  int load_file(std::string file, char *buffer);

  /** @todo document    */
  int save_file(std::string file, char *buffer);
};
