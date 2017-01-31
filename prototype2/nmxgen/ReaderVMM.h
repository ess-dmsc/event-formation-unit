/** Copyright (C) 2016 European Spallation Source ERIC */

#include <h5cc/H5CC_DataSet.h>
#include <h5cc/H5CC_File.h>

class ReaderVMM {
public:
  ReaderVMM(std::string filename);

  size_t read(char *buf);

private:
  H5CC::File file_;
  size_t total_{0};
  size_t current_{0};

  std::vector<hsize_t> slabsize{1, H5CC::kMax};
  std::vector<hsize_t> index{0, 0};

  H5CC::DataSet dataset_;
};
