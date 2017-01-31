/** Copyright (C) 2016 European Spallation Source ERIC */

#include <cassert>
#include <nmxgen/NMXArgs.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libs/include/Socket.h>
#include <unistd.h>
#include <h5cc/H5CC_File.h>
#include <h5cc/H5CC_DataSet.h>

// const int TSC_MHZ = 2900;

class ReaderVMM
{
public:
  ReaderVMM(std::string filename)
  {
    if (filename.empty())
      return;
    file_ = H5CC::File(filename, H5CC::Access::r_existing);
    if (!file_.has_group("RawVMM") || !file_.open_group("RawVMM").has_dataset("points"))
      return;
    dataset_ = file_.open_group("RawVMM").open_dataset("points");

    auto shape = dataset_.shape();
    if ((shape.rank() != 2) || (shape.dim(1) != 4))
      return;
      
    total_ = shape.dim(0);    
  }

  size_t read(char* buf)
  {
    //Event is timeoffset(32) timebin(16) planeID(8) stripnum(8) ADC(16)
    //  total = 10 bytes, unsigned
    // this should be quite close to the size of final data format
    
    size_t limit = std::min(current_ + 900, total_);
    size_t byteidx = 0;
    for (; current_ < limit; ++current_)
    {
      index[0] = current_;
      auto data = dataset_.read<uint32_t>(slabsize, index);
      buf[byteidx  ] = data.at(0);                                    //time offset (32 bits)
      buf[byteidx+4] = static_cast<uint16_t>(data.at(1));             //timebin (16 bits)
      buf[byteidx+6] = static_cast<uint8_t>(data.at(2) >> 16);        //planeid (8 bits)
      buf[byteidx+7] = static_cast<uint8_t>(data.at(2) & 0x000000FF); //strip   (8 bits)
      buf[byteidx+8] = static_cast<uint16_t>(data.at(3));             //adc value (16 bits)
      byteidx += 10;
    }
    return byteidx;
  }
  
private:
  H5CC::File file_;
  size_t total_{0};
  size_t current_{0};
  
  std::vector<hsize_t> slabsize {1,H5CC::kMax};
  std::vector<hsize_t> index    {0,0};
  
  H5CC::DataSet dataset_;
};


int main(int argc, char *argv[]) 
{
  NMXArgs opts(argc, argv);
  
  char buffer[9000];

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts.dest_ip.c_str(), opts.port);

  UDPClient DataSource(local, remote);
  DataSource.buflen(opts.buflen);
  DataSource.setbuffers(opts.sndbuf, 0);
  DataSource.printbuffers();

  ReaderVMM file(opts.filename);

  int readsz;

  uint64_t pkt = 0;
  uint64_t bytes = 0;

  while ((pkt < opts.txPkt) && ((readsz = file.read(buffer)) > 0)) 
  {
    DataSource.send(buffer, readsz);
    bytes += readsz;
    pkt++;

    usleep(opts.speed_level * 1000);
  }

  printf("Sent: %" PRIu64 " packets\n", pkt);
  printf("Sent: %" PRIu64 " bytes\n", bytes);

  return 0;
}
