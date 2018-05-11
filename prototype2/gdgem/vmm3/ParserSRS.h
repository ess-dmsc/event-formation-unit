/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to receive and generate Gd-GEM detector readout
 * from VMM2 ASICS via the SRS readout system
 */

#pragma once

#include <cinttypes>
#include <string.h>
#include <libs/include/BitMath.h>

static const int maximumNumberVMM{32};

class VMM3SRSData {
public:
  /**< Do NOT rearrange fields, used for casting to data pointer*/
  struct SRSHdr {
    uint32_t fc;     /**< frame counter packet field */
    uint32_t dataid; /**< data type identifier packet field */
    uint32_t txtime; /**< Transmission time for UDP packet */
  };

  struct VMM3Marker {
    uint32_t timeStamp;    /**< 32 bit */
    uint16_t triggerCount; /**< 10 bit */
  };

  struct VMM3Data {
    uint16_t bcid;         /**< 12 bit - bcid after graydecode */
    uint16_t adc;          /**< 10 bit - adc value from vmm readout */
    uint16_t triggerCounter;
    uint8_t tdc;           /**<  8 bit - tdc value from vmm readout */
    uint8_t chno;          /**<  6 bit - channel number from readout */
    uint8_t overThreshold; /**<  1 bit - over threshold flag for channel from readout */
    uint8_t vmmid;         /**<  5 bit - asic identifier - unique id per fec 0 - 15 */
    //uint8_t triggerOffset; /**<  5 bit - */
  };

  /** @brief create a data handler for VMM2 SRS data of fixed size Capacity
   * @param maxelements The maximum number of readout elements
   */
  VMM3SRSData(int maxelements) : max_elements(maxelements) {
    markers = new struct VMM3Marker[maximumNumberVMM];
    data = new struct VMM3Data[max_elements];

    memset(markers, 0, sizeof(struct VMM3Marker) * maximumNumberVMM);
  }

  ~VMM3SRSData() {
    delete[] data;
    data = 0;
    delete[] markers;
    markers = 0;
  }

  /** @brief reveive readouts from a binary payload buffer, return number of
   * data elements
   */
  int receive(const char *buffer, int size);

  /** @brief parse the readouts into a data array
   * @param data1 the raw (unbitreversed) data1 field of a SRS packet
   * @param data2 the raw (unbitreversed) data2 field of a SRS packet
   * @param vmd VMM2Data structure holding the parsed data (tdc, bcid, adc, ...)
   */
  int parse(uint32_t data1, uint16_t data2, struct VMM3Data *vmd);

  /// Holds data common to all readouts in a packet
  struct SRSHdr srshdr;

  /// holds all readout data in a packet (up to max_elems)
  struct VMM3Data *data{nullptr};

  /// holds time bases for all vmms in a readout
  struct VMM3Marker *markers{nullptr};

  // Results of the data parsing
  uint32_t elems{0}; // number of events
  uint32_t error{0}; // bytes of invalid data
  uint32_t timet0s{0};

  uint32_t max_elements{0}; // Maximum capacity of data array
};
