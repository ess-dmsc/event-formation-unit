/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief saves data to file
 */

 #include <stddef.h>
 #include <string>

class DataSave {
public:
  DataSave(std::string filename, void * data, size_t datasize);
  ~DataSave();
private:
  int fd;
};
