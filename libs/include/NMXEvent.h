

class NMXEvent {
public:
  NMXEvent(int detectorid, int timeval, int adc)
      : mDetectorId(detectorid), mTimeVal(timeval), mAdc(adc) {}
  bool operator<(NMXEvent other) const {
    return mDetectorId < other.getdetector();
  }
  bool operator>(NMXEvent other) const {
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
