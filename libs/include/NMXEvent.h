#include <vector>

class NMXData {
public:
  NMXData(int detectorid, int timeval, int adc)
      : detectorId_(detectorid), timeVal_(timeval), adc_(adc) {}
  bool operator<(NMXData other) const {
    return detectorId_ < other.getdetector();
  }
  bool operator>(NMXData other) const {
    return detectorId_ > other.getdetector();
  }
  int getdetector() { return detectorId_; }
  int gettime() { return timeVal_; }
  int getadc() { return adc_; }

private:
  int detectorId_;
  int timeVal_;
  int adc_;
};

/** BulkData is just an array of NMXData */
class VMMBulkData {
public:
  void add(NMXData d) { data.emplace_back(d); }

private:
  std::vector<NMXData> data;
  ;
};
