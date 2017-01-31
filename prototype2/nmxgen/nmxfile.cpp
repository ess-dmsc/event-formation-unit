/** Copyright (C) 2016 European Spallation Source ERIC */

#include <cassert>
#include <nmxgen/NMXArgs.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <numeric>
#include <limits>
#include <set>
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

struct EntryNMX
{
  uint64_t time;
  uint8_t  plane_id;
  uint8_t  strip;
  uint16_t adc;
};

struct PlaneNMX
{
  void push(const EntryNMX& e)
  {
    if (entries.empty())
      time_start = time_end = e.time;
    entries.push_back(e);
    integral += e.adc;
    time_start = std::min(time_start, e.time);
    time_end = std::max(time_start, e.time);
  }
  
  void analyze(bool weighted, uint16_t max_timebins, uint16_t max_timedif)
  {
    double center_sum {0};
    double center_count {0};
    int16_t lspan_min = std::numeric_limits<int16_t>::max();
    int16_t lspan_max = std::numeric_limits<int16_t>::min();
    int16_t uspan_min = std::numeric_limits<int16_t>::max();
    int16_t uspan_max = std::numeric_limits<int16_t>::min();
    uint64_t earliest = time_end - max_timedif;
    std::set<uint64_t> timebins;
    for (const auto& e : entries)
    {
      timebins.insert(e.time);
      if (e.time == time_end)
      {
        center_count++;
        if (weighted)
          center_sum += e.strip * e.adc;
        else
          center_sum += e.strip;
        lspan_min = std::min(lspan_min, static_cast<int16_t>(e.strip));
        lspan_max = std::max(lspan_max, static_cast<int16_t>(e.strip));
      }
      if ((timebins.size() < max_timebins) && (e.time >= earliest))
      {
        uspan_min = std::min(uspan_min, static_cast<int16_t>(e.strip));
        uspan_max = std::max(uspan_max, static_cast<int16_t>(e.strip));        
      }
    }
    center = center_sum / center_count;
    uncert_lower = lspan_max - lspan_min;
    uncert_upper = uspan_max - uspan_min;
  } 

  double center;
  int16_t uncert_lower, uncert_upper;

  uint64_t time_start, time_end;
  double integral;
  
  std::list<EntryNMX> entries;
};

struct EventNMX
{
  void push(const EntryNMX& e)
  {
    if (e.plane_id)
      y.push(e);
    else
      x.push(e);
  }
  
  void analyze(bool weighted, int16_t max_timebins, int16_t max_timedif)
  {
    if (x.entries.size())
      x.analyze(weighted, max_timebins, max_timedif);
    if (y.entries.size())
      y.analyze(weighted, max_timebins, max_timedif);
    good = x.entries.size() && y.entries.size();
  }
  
  bool good {false};
  PlaneNMX x, y;
};

class ParserClusterer
{
public:
  void parse (char* buf, size_t size)
  {
    size_t limit = std::min(size / 10, size_t(900));
    size_t byteidx = 0;
    for (; byteidx < limit; ++byteidx)
    {
      EntryNMX entry;
      entry.time = *(buf + byteidx);
      entry.time = entry.time << 32;
      entry.time += *(buf + byteidx + 4); //time offset (32 bits) + timebin (16 bits)
      entry.plane_id = *(buf + byteidx + 6);                         //planeid (8 bits)
      entry.strip = *(buf + byteidx + 7);
      entry.adc = *(buf + byteidx + 8);
      byteidx += 10;
      backlog_.insert(std::pair<uint64_t, EntryNMX>(entry.time, entry));
    }
  }
  
  bool event_ready()
  {
    return (!backlog_.empty() &&
            ((backlog_.rbegin()->first - backlog_.begin()->first) > 30));
  }
  
  EventNMX get()
  {
    if (!event_ready())
      return EventNMX();
    EventNMX ret;
    auto latest = backlog_.begin()->first + 30;
    while (backlog_.begin()->first <= latest)
    {
      ret.push(backlog_.begin()->second);
      backlog_.erase(backlog_.begin());
    }
    return ret;
  }
  
private:
  std::multimap<uint64_t, EntryNMX> backlog_;
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
