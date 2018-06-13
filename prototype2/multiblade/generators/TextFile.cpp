//
// Created by troels on 6/2/17.
//

/// GCOVR_EXCL_START

#include "TextFile.h"

TextFile::TextFile(std::string fileName) {
  file.open(fileName);
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
}

/*
 * Returns next entry.
 * Throws eofEx exception if end of file is reached i.e. no more entries can be
 * retrieved.
 */
TextFile::Entry TextFile::nextEntry() {
  double t, d, c, a;
  try {
    file >> t >> d >> c >> a;
  } catch (std::exception e) {
    if (file.eof())
      throw eofEx;
    else
      throw e;
  }
  Entry entry(t, a, d, c);
  return entry;
}

/*
 * Returns next n entries.
 * If EOF is reached before n entries is read all entries until EOF is returned.
 */
std::vector<TextFile::Entry> TextFile::nextEntries(size_t n) {
  std::vector<Entry> result;
  for (size_t i = 0; i < n; ++i) {
    try {
      result.push_back(nextEntry());
    } catch (eof e) {
      break;
    }
  }
  return result;
}

/*
 * Returns all entries left in the file
 */
std::vector<TextFile::Entry> TextFile::rest() {
  std::vector<Entry> result;
  while (true) {
    try {
      result.push_back(nextEntry());
    } catch (eof e) {
      break;
    }
  }
  return result;
}

/*
 *
 */
size_t TextFile::nextChunk(TextFile::Entry *buf, size_t size) {
  size_t i = 0;
  while ((i + 1) * sizeof(Entry) <= size) {
    try {
      buf[i++] = nextEntry();
    } catch (eof e) {
      break;
    }
  }
  return i * sizeof(Entry);
}
/// GCOVR_EXCL_STOP
