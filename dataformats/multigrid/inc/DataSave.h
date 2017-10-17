/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief saves data to file
 */

#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

class DataSave {
public:
  /** @todo document */
  DataSave(std::string filename, void *data, size_t datasize);

  /** @todo document */
  DataSave(std::string filename);

  /** @todo document */
  DataSave(std::string filename_prefix, int addunixtime);

  /** @todo document */
  int tofile(std::string);

  int tofile(char *buffer, size_t len);

  /** printf-like formatting */
  int tofile(const char * fmt,...);

  /** @todo document */
  ~DataSave();

private:
  int fd{-1};

  const int flags = O_TRUNC | O_CREAT | O_WRONLY;
  const int mode = S_IRUSR | S_IWUSR;
};
