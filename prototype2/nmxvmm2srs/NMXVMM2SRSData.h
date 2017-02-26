/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to receive and generate CSPEC detector readout
 */

#pragma once

class NMXVMM2SRSData {
public:

  NMXVMM2SRSData(){};

  /** @brief parse a binary payload buffer, return number of data elements
   */
  int receive(const char *buffer, int size);

private:
  unsigned int frame_counter{0};
  unsigned int dataid{0};
  unsigned int errbytes{0};
};
