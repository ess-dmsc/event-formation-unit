/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to receive and generate Gd-GEM detector readout
 * from VMM2 ASICS via the SRS readout system
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
    /**< @todo flags? */
  };

  /** @brief create a data handler for VMM2 SRS data of fixed size Capacity
   * @param maxelements The maximum number of readout elements
   */
  NMXVMM2SRSData(int maxelements) : max_elements(maxelements) {
    data = new struct VMM2Data[max_elements];
    hist_clear();
  };

  ~NMXVMM2SRSData() {
    delete[] data;
    data = 0;
  };

  /** @brief reveive readouts from a binary payload buffer, return number of
   * data elements
   */
  int receive(const char *buffer, int size);

  /** @brief parse the readouts into a data array
   * @param data1 the raw (unbitreversed) data1 field of a SRS packet
   * @param data2 the raw (unbitreversed) data2 field of a SRS packet
   * @param vmd VMM2Data structure holding the parsed data (tdc, bcid, adc, ...)
   */
  int parse(uint32_t data1, uint32_t data2, struct VMM2Data *vmd);

  /** @brief clears histograms for x and y strips */
  void hist_clear();

  struct SRSHdr srshdr; /**< Holds data common to all readouts in a packet */
  struct VMM2Data *data{nullptr}; /**< holds all readout data in a packet (up to max_elems) */

  // Results of the data parsing
  uint32_t elems{0}; // number of events
  uint32_t error{0}; // bytes of invalid data

  uint32_t max_elements{0}; // Maximum capacity of data array
  uint32_t xyhist[2][1500];
  uint32_t xyhist_elems{0};
private:
  uint32_t reversebits(uint32_t data); /**< helper function to reverse bits in a uint32_t */
  unsigned int gray2bin32(unsigned int num); /**< helper function to decode gray codes */
};
