/** Copyright (C) 2016 European Spallation Source */

#include <cinttypes>

class CSPECData {
public:
  // See "multi grid detector data processing.docx" from DMG's Event Formation
  // Files
  const unsigned int wire_thresh = 230; // From Anton 4 oct 2016
  const unsigned int grid_thresh = 170; //  ...

  const int datasize = 40; /**< size (bytes) of a data readout */
  // clang-format off
  const unsigned int header_mask = 0xc0000000;
  const unsigned int header_id =   0x40000000;
  const unsigned int data_id =     0x00000000;
  const unsigned int footer_id =   0xc0000000;
  const unsigned int nwords = 9;
  // clang-format on

  struct mgd { // multi grid data
    unsigned int module;
    unsigned int d[8];
    unsigned int time;
  };

  /** parse a binary payload buffer, return number of data elements */
  int receive(const char *buffer, int size);

  /** Discard data below threshold, double events, etc., return number
  *  of discarded samples */
  int input_filter();

  /** Generate simulated data, place in user specified buffer */
  int generate(char *buffer, int size, int elems);

  // This data is overwritten on receive()
  struct mgd data[250];
  unsigned int elems{0};
  unsigned int error{0};
  unsigned int frag{0};
};
