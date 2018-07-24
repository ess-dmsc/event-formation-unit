/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class to receive and generate Gd-GEM detector readout
/// from VMM2 ASICS via the SRS readout system
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <cstddef>
#include <limits>

class NMXVMM2SRSData {
  static constexpr uint32_t max_frame_counter {std::numeric_limits<uint32_t>::max() / 2};

public:
  /**< Do NOT rearrange fields, used for casting to data pointer*/
  struct SRSHdr {
    uint32_t fc;     /**< frame counter packet field */
    uint32_t dataid; /**< data type identifier packet field */
    uint32_t time;   /**< timestamp packet field */
  };

  struct VMM2Data {
    uint16_t bcid;          /**< bcid after graydecode */
    uint16_t tdc;           /**< tdc value from vmm readout */
    uint16_t adc;           /**< adc value from vmm readout */
    uint16_t chno;          /**< channel number from readout */
    uint16_t overThreshold; /**< over threshold flag for channel from readout */
    /**< @todo flags? */
  };

  /** \brief create a data handler for VMM2 SRS data of fixed size Capacity
   * @param maxelements The maximum number of readout elements
   */
  NMXVMM2SRSData(size_t maxelements) : max_elements(maxelements) {
    data = new struct VMM2Data[max_elements];
  }

  ~NMXVMM2SRSData() {
    delete[] data;
    data = 0;
  }

  /** \brief reveive readouts from a binary payload buffer, return number of
   * data elements
   */
  int receive(const char *buffer, int size);

  /** \brief parse the readouts into a data array
   * @param data1 the raw (unbitreversed) data1 field of a SRS packet
   * @param data2 the raw (unbitreversed) data2 field of a SRS packet
   * @param vmd VMM2Data structure holding the parsed data (tdc, bcid, adc, ...)
   */
  int parse(uint32_t data1, uint32_t data2, struct VMM2Data *vmd);

  struct SRSHdr srshdr; /**< Holds data common to all readouts in a packet */
  struct VMM2Data *data{
      nullptr}; /**< holds all readout data in a packet (up to max_elems) */

  // Results of the data parsing
  size_t elems{0}; // number of events
  size_t error{0}; // bytes of invalid data
  size_t stats_bcid_tdc_errors {0};

  size_t max_elements{0}; // Maximum capacity of data array

  uint32_t old_frame_counter {0};
  uint32_t old_time {0};
  uint32_t time_bonus {0};

  inline bool frame_counter_overflow(uint32_t old_framecounter,
                                     uint32_t framecounter)
  {
    return (old_framecounter > (framecounter + max_frame_counter));
  }


private:
  /// helper function to reverse bits in a uint32_t
  uint32_t reversebits(uint32_t data);

  /// helper function to decode gray codes
  unsigned int gray2bin32(unsigned int num);
};
