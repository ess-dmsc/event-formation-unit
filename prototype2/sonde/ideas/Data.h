/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to receive SoNDe data from IDEAS readoout
 */

#pragma once

#include <cinttypes>

class IDEASData {
public:

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
