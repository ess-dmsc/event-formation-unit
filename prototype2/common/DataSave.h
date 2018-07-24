/// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief saves data to file
///
//===----------------------------------------------------------------------===//

#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

class DataSave {
public:
  /** @brief write buffer to file
   * @param filename filename which can include path
   * @param data pointer to data buffere
   * @param datasize number of bytes to write
   */
  DataSave(std::string filename, void *data, uint64_t datasize);

  /** @brief create file for later writing
   * @param filename filename which can include path
   */
  DataSave(std::string filename);

  /** @brief create file for later writing, specify max file size before
   * splitting
   * @param fileprefix can include path, filename will be generated
   * @param maxfilesize split file into chunks of this size, adds sequence
   * number
   */
  DataSave(std::string fileprefix, uint64_t maxfilesize);

  /** @brief write string to file, not buffered, slow */
  int tofile(std::string);

  /** @brief write buffer of size len to file */
  int tofile(char *buffer, uint64_t len);

  /** @brief printf-like formatting using buffered writes
   * for better performance
   */
  int tofile(const char *fmt, ...);

  /** @brief return the filename of the current file (buffered write only) */
  std::string getfilename();

  /** @brief closes file descriptor */
  ~DataSave();

private:
#define BUFFERSIZE 20000
#define MARGIN 2000
  char buffer[BUFFERSIZE + MARGIN];
  int bufferlen{0};

  int fd{-1};                      /**< unix file descriptor for savefile */
  int sequence_number{1};          /**< filename sequence number */
  std::string filename_prefix{""}; /**< base filename */
  std::string startTime{""};       /**< start time common to all files */
  uint64_t curfilesize{0};         /**< bytes written to file */
  uint64_t maxfilesize{50000000};  /**< create new sequence number after maxfilesize bytes */

  const int flags = O_TRUNC | O_CREAT | O_RDWR;
  const int mode = S_IRUSR | S_IWUSR;

  /** @brief figure out if a new file needs to be created */
  int adjustfilesize(int bytes);

  /** @brief close old file, create new (with new sequence number) */
  void createfile();
};
