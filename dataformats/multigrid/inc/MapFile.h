/// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for anstraction of mmap()
///
//===----------------------------------------------------------------------===//

#pragma once

#include <string>

class MapFile {
public:
  /** @brief mmap a filename
   *  @filename File to be mmap'ed
   */
  MapFile(std::string filename);

  /** @brief descructor handles cleanup
   */
  ~MapFile();

  /** @brief get the address of the mmap'ed file
   */
  const char *getaddress();

  /** @brief get the size of the mmap'ed file
   */
  size_t getsize();

private:
  void *address{nullptr}; /**< address of mmap'ed file */
  size_t filesize{0};     /**< size of mmap'ed file */
  int fd{-1};             /**< file descriptor used for mmap/munmap */
};
