/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for mmap'ing a file
 */

#pragma once

#include <string>

class MapFile {
public:
  /** @todo document
   */
  MapFile(std::string filename);

  /** @todo document
   */
  ~MapFile();

  /** @todo document
   */
  const char *getaddress();

  /** @todo document
   */
  size_t getsize();

private:
  void *address{nullptr}; /**< address of mmap'ed file */
  size_t filesize;
  int fd;
};
