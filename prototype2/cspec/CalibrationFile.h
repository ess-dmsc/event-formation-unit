/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Load CSPEC wire and grid calibratyion from file
 *  @todo no array bounds checking is done for buffers passed as ptrs.
 */

#pragma once

#include <string>

class CalibrationFile {
public:
  /** @brief load a multigrid calibration from file to array
   *  @param[in] calibration filename prefix. Ex. to load a.wcal and a.gcal
   *   set calibration = "a"
   *  @param[out] wirecal array to hold data assumed to be of size
   *   CSPECChanConv::adcsize * 2
   *  @param[out] gridcal array to hold data assumed to be of size
   *   CSPECChanConv::adcsize * 2
   */
  int load(std::string calibration, char *wirecal, char *gridcal);

  /** @brief save calibration files from array
   *  @param[out] calibration filename prefix. Ex. to save to a.wcal and a.gcal
   *   set calibration = "a"
   *  @param[in] wirecal array to hold data assumed to be of size
   *   CSPECChanConv::adcsize * 2
   *  @param[in] gridcal array to hold data assumed to be of size
   *   CSPECChanConv::adcsize * 2
   */
  int save(std::string calibration, char *wirecal, char *gridcal);

private:
  /** @brief load contents of file into buffer
   *  @param file filename to load from
   *  @param buffer Buffer is assumed to be of size CSPECChanConv::adcsize * 2
   */
  int load_file(std::string file, char *buffer);

  /** @brief save contents of buffer into file.
   *  @param file filename to save to
   *  @param buffer Buffer is assumed to be of size CSPECChanConv::adcsize * 2
   */
  int save_file(std::string file, char *buffer);
};
