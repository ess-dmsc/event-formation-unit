/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief ESS Readout System Data definitions and parsing functions
 */

#pragma once

#include <cinttypes>

class Readout {
public:
  enum error { OK = 0, EBUFFER, ESIZE, EPAD, EHDR };

  uint16_t type;      // overwritten on eache receive()
  uint16_t wordcount; //  -=-
  uint16_t seqno;     //  -=-
  uint16_t reserved;  //  -=-

  struct Payload {
    uint16_t type;
    uint16_t wordcount;
    uint16_t seqno;
    uint16_t reserved;
  } __attribute__((packed));

  /** @brief validate a readout buffer
  *  @param[in] buffer pointer to data
  *  @param[in] size length of buffer in bytes
  *  @return on success return 0, else -1
  */
  int validate(const char *buffer, uint32_t size);
};

static_assert(sizeof(Readout::Payload) == 8,
              "Wrong header size (update assert or check packing)");
