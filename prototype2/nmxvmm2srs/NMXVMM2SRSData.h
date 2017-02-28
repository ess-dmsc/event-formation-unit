/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to receive and generate CSPEC detector readout
 */

#pragma once

#include <cinttypes>

class NMXVMM2SRSData {
public:

  struct SRSHdr {
    uint32_t fc;
    uint32_t dataid;
    uint32_t time;
  };

  struct VMM2Data {
    uint16_t tdc;
    uint16_t adc;
    uint16_t chno;
    uint16_t bcid;
  };

  NMXVMM2SRSData(int maxevents){
    data = new struct VMM2Data[maxevents];
  };

  ~NMXVMM2SRSData() {
    delete [] data;
    data = 0;
  };

  /** @brief parse a binary payload buffer, return number of data elements
   */
  int receive(const char *buffer, int size);

  /** @todo document */
  int parse(uint32_t data1, uint32_t data2, struct VMM2Data * vmd);


  struct SRSHdr srshdr; // Common to all readouts in a packet
  struct VMM2Data * data{nullptr};

  // Results of the data parsing
  uint32_t elems{0}; // number of events
  uint32_t error{0}; // bytes of invalid data

private:
  uint32_t reverse(uint32_t data);
  unsigned int grayToBinary32(unsigned int num);
};
