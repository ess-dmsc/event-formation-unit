/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to receive and generate CSPEC detector readout
 */

#pragma once

#include <cinttypes>

class NMXVMM2SRSData {
public:

  /**< Do NOT rearrange fields, used for casting to data pointer*/
  struct SRSHdr {
    uint32_t fc;     /**< frame counter packet field */
    uint32_t dataid; /**< data type identifier packet field */
    uint32_t time;   /**< timestamp packet field */
  };

  struct VMM2Data {
    uint16_t bcid; /**< bcid after graydecode */
    uint16_t tdc;  /**< tdc value from vmm readout */
    uint16_t adc;  /**< adc value from vmm readout */
    uint16_t chno; /**< channel number from readout */
  };

  NMXVMM2SRSData(int maxelements) : max_elements(maxelements){
    data = new struct VMM2Data[max_elements];
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
  uint32_t max_elements{0}; // Capacity of data array

private:
  uint32_t reversebits(uint32_t data);
  unsigned int gray2bin32(unsigned int num);
};
