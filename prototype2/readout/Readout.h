/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief ESS Readout System Data definitions and parsing functions
 */

#pragma once

#include <cinttypes>

class Readout {
public:
   uint16_t type;       // overwritten on eache receive()
   uint16_t wordcount;  //  -=-
   uint16_t seqno;      //  -=-
   uint16_t reserved;   //  -=-

   struct Payload {
       uint16_t type;
       uint16_t wordcount;
       uint16_t seqno;
       uint16_t reserved;
   } __attribute__((packed));

   int receive(const char *buffer, int size);
};

static_assert(sizeof(Readout::Payload) == 8, "Wrong header size (update assert or check packing)");
