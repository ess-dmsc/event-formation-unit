/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#ifndef MBUDPGEN_TEXTFILE_H
#define MBUDPGEN_TEXTFILE_H

#include <cstdint>
#include <exception>
#include <fstream>
#include <vector>

namespace Multiblade {

class TextFile {
private:
  std::ifstream file;

public:
  struct Entry {
    uint32_t time;
    uint16_t adc;
    uint8_t digi;
    uint8_t chan;
    Entry(double t, double a, double d, double c)
        : time((uint32_t) t), adc((uint16_t) a), digi((uint8_t) d),
          chan((uint8_t) c) {}
  };
  class eof : public std::exception {
    virtual const char *what() const throw() { return "End of file reached"; }
  } eofEx;
  TextFile(std::string fileName);
  Entry nextEntry();
  size_t nextChunk(Entry *buf, size_t size);
  std::vector<Entry> nextEntries(size_t n);
  std::vector<Entry> rest();
};

#endif // MBUDPGEN_TEXTFILE_H
// GCOVR_EXCL_STOP

}
