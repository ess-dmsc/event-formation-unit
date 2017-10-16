/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to receive SoNDe data from IDEAS readoout
 */

#pragma once

#include <cinttypes>
#include <sonde/Geometry.h>

class IDEASData {
public:

  struct SoNDeData {
    uint32_t time;
    uint32_t pixel_id;
  };

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
  IDEASData(SoNDeGeometry * geom, int filedescriptor) : sondegeometry(geom), fd(filedescriptor){ }

  ~IDEASData() { }

  /** @brief parse a binary payload buffer, return number of data elements
   */
  int receive(const char *buffer, int size);

  struct SoNDeData data[500];
  unsigned int events{0};  /**< number of valid events */
  unsigned int errors{0};  /**< nuber of geometry errors in readout */
private:
  SoNDeGeometry * sondegeometry{nullptr};
  int fd;
};
