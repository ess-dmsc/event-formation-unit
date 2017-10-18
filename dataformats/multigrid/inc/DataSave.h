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

  /** @brief write string to file */
  int tofile(std::string);

  /** @brief write buffer of size len to file */
  int tofile(char *buffer, size_t len);

  /** @brief printf-like formatting */
  int tofile(const char * fmt,...);

  /** @brief closes file descriptor */
  ~DataSave();

private:
  int fd{-1};                      /**< unix file descriptor for savefile */
  int sequence_number{1};          /**< filename sequence number */
  std::string filename_prefix{""}; /**< base filename */
  uint32_t curfilesize{0};         /**< bytes written to file */
  uint32_t maxfilesize{50000000};   /**< create new sequence number after maxfilesize bytes */

  const int flags = O_TRUNC | O_CREAT | O_WRONLY;
  const int mode = S_IRUSR | S_IWUSR;

  /** @brief figure out if a new file needs to be created */
  int adjustfilesize(int bytes);
  /** @brief close old file, create new (with new sequence number) */
  void createfile();
};
