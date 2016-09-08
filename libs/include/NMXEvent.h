#include <vector>

class NMXData {
public:
  NMXData(int detectorid, int timeval, int adc)
      : mDetectorId(detectorid), mTimeVal(timeval), mAdc(adc) {}
  bool operator<(NMXData other) const {
    return mDetectorId < other.getdetector();
  }
  bool operator>(NMXData other) const {
    return mDetectorId > other.getdetector();
  }
  int getdetector() { return mDetectorId; }
  int gettime() { return mTimeVal; }
  int getadc() { return mAdc; }

private:
  int mDetectorId;
  int mTimeVal;
  int mAdc;
};


/** BulkData is just an array of NMXData */
class BulkData {
 public:
  void add(NMXData d) {
    data.emplace_back(d);
  }
 private:
  std::vector<NMXData> data;;
};
