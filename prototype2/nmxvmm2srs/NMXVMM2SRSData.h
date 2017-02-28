/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to receive and generate CSPEC detector readout
 */

#pragma once

#include <cinttypes>

  struct VMM2Data {
    uint16_t tdc;
    uint16_t adc;
    uint32_t time;
  };

class NMXVMM2SRSData {
public:

  NMXVMM2SRSData(int maxevents){
    data = new struct VMM2Data[maxevents];
  };

  ~NMXVMM2SRSData(){
    delete [] data;
    data = 0;
  };

  /** @brief parse a binary payload buffer, return number of data elements
   */
  int receive(const char *buffer, int size);


  // Fields common to all readouts
  uint32_t frame_counter{0};
  uint32_t timestamp{0};

  struct VMM2Data * data{nullptr};
  uint32_t elems{0}; // number of events
  uint32_t error{0}; // bytes of invalid data
};
