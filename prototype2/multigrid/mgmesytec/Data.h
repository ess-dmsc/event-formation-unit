/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to parse detector readout for multigrid via
 * sis3153 / Mesytec digitizer
 */

#pragma once
#include <common/ESSGeometry.h>
#include <common/ReadoutSerializer.h>
#include <dataformats/multigrid/inc/DataSave.h>
#include <multigrid/mgmesytec/MG24Detector.h>

class MesytecData {
public:
  enum error { OK = 0, ESIZE, EHEADER, EUNSUPP };

  // clang-fomat off
  // sis3153 and mesytec data types from
  // Struck: mvme-src-0.9.2-281-g1c4c24c.tar
  // Struck: Ethernet UDP Addendum revision 107
  // Mesytec Datasheet: VMMR-8/16 v00.01
  enum DataType {
    mesytecData = 0x10000000,
    mesytecTimeOffset = 0x20000000,
    mesytecHeader = 0x40000000,
    mesytecTimeStamp = 0xc0000000,
    sisBeginReadout = 0xbb000000,
    sisEndReadout = 0xee000000
  };
  // clang-fomat on

  MesytecData() {
#ifdef DUMPTOFILE
    mgdata.tofile("#Time, Bus, Address, ADC\n");
#endif
  };

  ~MesytecData(){};

  int getPixel(); // @todo (too) simple implm. but agreed for now
  int getTime();  // @todo (too) simple implm. but agreed for now

  void setWireThreshold(int low, int high) {
     wireThresholdLo = low;
     wireThresholdHi = high;
   };

  void setGridThreshold(int low, int high) {
     gridThresholdLo = low;
     gridThresholdHi = high;
   };
  /** @brief parse a binary payload buffer, return number of data element
   * @todo Uses NMXHists  - refactor and move ?
   */
  int parse(const char *buffer, int size, NMXHists &hists, ReadoutSerializer &serializer);

  /** @brief parse n 32 bit words from mesytec VMMR-8/16 card */
  void mesytec_parse_n_words(uint32_t *buffer, int nWords, NMXHists &hists, ReadoutSerializer &serializer);

  int readouts{0}; /**< number of channels read out */
  int discards{0};

  int wiremax{-1}; // initial alg.: wire with max adc
  int gridmax{-1}; // initial alg.: grid with max adc
  int time{-1};

private:
  int wireThresholdLo{0};
  int wireThresholdHi{65535};
  int gridThresholdLo{0};
  int gridThresholdHi{65535};
  MG24Detector mgseq;
  ESSGeometry mg{4, 48, 20, 1};

#ifdef DUMPTOFILE
  DataSave mgdata{std::string("multigrid_mesytec_"), 100000000};
#endif
};
