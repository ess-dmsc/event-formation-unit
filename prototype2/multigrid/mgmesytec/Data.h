/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to receive and generate CSPEC detector readout
 */

#pragma once
#include <dataformats/multigrid/inc/DataSave.h>

class MesytecData {
public:
  enum error { OK = 0, ESIZE, EHEADER, EUNSUPP};

  // clang-fomat off
  // sis3153 and mesytec data types from
  // Struck: mvme-src-0.9.2-281-g1c4c24c.tar
  // Struck: Ethernet UDP Addendum revision 107
  // Mesytec Datasheet: VMMR-8/16 v00.01
  enum DataType {
    mesytecData =       0x10000000,
    mesytecTimeOffset = 0x20000000,
    mesytecHeader  =    0x40000000,
    mesytecTimeStamp =  0xc0000000,
    sisBeginReadout =   0xbb000000,
    sisEndReadout =     0xee000000
  };
  // clang-fomat on

  MesytecData() {

#ifdef DUMPTOFILE
    mgdata.tofile("#Time, Bus, Address, ADC\n");
#endif
  };

  ~MesytecData() {};

  /** @brief parse a binary payload buffer, return number of data element */
  int parse(const char *buffer, int size);

  /** @brief parse n 32 bit words from mesytec VMMR-8/16 card */
  void mesytec_parse_n_words(uint32_t * buffer, int nWords);

  int readouts{0};  /**< number of channels read out */

private:
#ifdef DUMPTOFILE
  DataSave mgdata{std::string("multigrid_mesytec_"), 100000000};
#endif
};
