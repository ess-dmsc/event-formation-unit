/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief saves data to file
 */

#include <stddef.h>
#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

class DataSave {
public:
  /** @todo document */
  DataSave(std::string filename, void *data, size_t datasize);

  /** @todo document */
  DataSave(std::string);

  /** @todo document */
  int tofile(std::string);

  int tofile(char * buffer, size_t len);

  /** @todo document */
  ~DataSave();

private:
  int fd{-1};

  const int flags = O_TRUNC | O_CREAT | O_WRONLY;
  const int mode = S_IRUSR | S_IWUSR;
};
