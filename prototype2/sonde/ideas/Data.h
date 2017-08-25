/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to receive SoNDe data from IDEAS readoout
 */

#pragma once

#include <cinttypes>

class IDEASData {
public:

  /** from IDEAS Readout and Control Packet Protocol Reference
   * Ref: IDE-REP-Ref-V1.7
   * Date: 2017-05-23
   * Direction: System -> PC
   */
  struct Header {
    uint16_t id;
    uint16_t pktseq;
    uint32_t timestamp;
    uint16_t length;
  } __attribute__((packed));

  /** Empty constructor */
  IDEASData() { }

  ~IDEASData() { }

  /** @brief parse a binary payload buffer, return number of data elements
   */
  int receive(const char *buffer, int size);

  /** @brief serialize event to buffer
   *  @param buffer User specified buffer (must be large enough to hold event
   *  @todo document return value
   */
  int createevent(uint32_t time, uint32_t pixel_id, char *buffer);

};
