/** Copyright (C) 2016 European Spallation Source */

#include <cinttypes>
#include <queue>

class CSPECData {
public:
  // See "multi grid detector data processing.docx" from DMG's Event Formation
  // Files
  static const int datasize = 40;
  static const unsigned int header_mask = 0xc0000000;
  static const unsigned int header_id = 0x40000000;
  static const unsigned int data_id = 0x00000000;
  static const unsigned int footer_id = 0xc0000000;
  static const unsigned int nwords = 9;

  struct mgd { // multi grid data
    unsigned int module;
    unsigned int d[8];
    unsigned int time;
  };

  /** parse a binary payload buffer, return number of data elements */
  int receive(const char *buffer, int size);

  /** Generate simulated data, place in user specified buffer */
  int generate(char *buffer, int size, int elems);

  struct mgd data[250];
  uint64_t ierror{0};
  uint64_t idata{0};
  uint64_t ifrag{0};
};
