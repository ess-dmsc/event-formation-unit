#include <algorithm>
#include <cmath>
#include <cassert>
#include <common/Trace.h>
#include <gdgem/dg_impl/NMXClusterer.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

HitsQueue::HitsQueue(SRSTime Time, double deltaTimeHits)
    : pTime(Time), pDeltaTimeHits(deltaTimeHits) {}

const HitContainer& HitsQueue::hits() const
{
  return hitsOut;
}

void HitsQueue::store(uint16_t strip, uint16_t adc, double chipTime) {
  if (chipTime < pTime.max_chip_time_in_window()) {
    hitsNew.emplace_back(Eventlet());
    auto &e = hitsNew[hitsNew.size() - 1];
    e.adc = adc;
    e.strip = strip;
    e.time = chipTime;
  } else {
    hitsOld.emplace_back(Eventlet());
    auto &e = hitsOld[hitsOld.size() - 1];
    e.adc = adc;
    e.strip = strip;
    e.time = chipTime;
  }
}

void HitsQueue::sort_and_correct()
{
  std::sort(begin(hitsOld), end(hitsOld),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.time <= e2.time;
            });

  std::sort(begin(hitsNew), end(hitsNew),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.time <= e2.time;
            });
  CorrectTriggerData();

  hitsOut = std::move(hitsOld);

  hitsOld = std::move(hitsNew);
  if (!hitsNew.empty()) {
    hitsNew.clear();
  }
}

void HitsQueue::subsequentTrigger(bool trig)
{
  m_subsequentTrigger = trig;
}

void HitsQueue::CorrectTriggerData() {
  if (!m_subsequentTrigger)
    return;

  const auto &itHitsBegin = begin(hitsNew);
  const auto &itHitsEnd = end(hitsNew);
  const auto &itOldHitsBegin = hitsOld.rend();
  const auto &itOldHitsEnd = hitsOld.rbegin();

  // If either list is empty
  if (itHitsBegin == itHitsEnd || itOldHitsBegin == itOldHitsEnd)
    return;

  double timePrevious = itOldHitsEnd->time; // Newest of the old
  // oldest of the new + correct into time space of the old
  double timeNext = itHitsBegin->time + pTime.trigger_period();
  double deltaTime = timeNext - timePrevious;
  //Continue only if the first hit in hits is close enough in time to the last hit in oldHits
  if (deltaTime > pDeltaTimeHits)
    return;

  HitContainer::iterator itFind;
  //Loop through all hits in hits
  for (itFind = itHitsBegin; itFind != itHitsEnd; ++itFind) {
    //At the first iteration, timePrevious is sett to the time of the first hit in hits
    timePrevious = timeNext;
    //At the first iteration, timeNext is again set to the time of the first hit in hits
    // + correct into time space of the old
    timeNext = itFind->time + pTime.trigger_period();

    //At the first iteration, delta time is 0
    deltaTime = timeNext - timePrevious;

    if (deltaTime > pDeltaTimeHits)
      break;

    hitsOld.emplace_back(Eventlet());
    auto &e = hitsNew[hitsNew.size() - 1];
    e.adc = itFind->adc;
    e.strip = itFind->strip;
    e.time = timeNext;
  }

  //Deleting all hits that have been inserted into oldHits (up to itFind, but not including itFind)
  hitsNew.erase(itHitsBegin, itFind);
}



NMXHitSorter::NMXHitSorter(SRSTime time, SRSMappings chips, uint16_t ADCThreshold,
                           double deltaTimeHits, NMXClusterer& cb) :
    hits(pTime, deltaTimeHits),
    pTime(time), pChips(chips), pADCThreshold(ADCThreshold), callback_(cb)
{

}


//====================================================================================================================
void NMXHitSorter::AnalyzeHits(int triggerTimestamp, unsigned int frameCounter,
                               int fecID, int vmmID, int chNo, int bcid, int tdc, int adc,
                               int overThresholdFlag) {

  // Ready for factoring out, logic tested elsewhere
  uint16_t strip = pChips.get_strip(fecID, vmmID, chNo);

  // These variables are used only here
  // Block is candidate for factoring out
  // Perhaps an adapter class responsible for recovery from this error condition?

  // Fix for entries with all zeros
  if (bcid == 0 && tdc == 0 && overThresholdFlag) {
    bcid = m_oldBcid;
    tdc = m_oldTdc;
    stats_bcid_tdc_error++;
  }
  m_oldBcid = bcid;
  m_oldTdc = tdc;

  // Could be factored out depending on above block
  double chipTime = pTime.chip_time(bcid, tdc);

  bool newEvent = false;
  double deltaTriggerTimestamp_ns = 0;
  double triggerTimestamp_ns = pTime.timestamp_ns(triggerTimestamp);

  if (m_oldTriggerTimestamp_ns != triggerTimestamp_ns) {
    AnalyzeClusters();
    newEvent = true;
    hits.subsequentTrigger(false);
    m_eventNr++;
    deltaTriggerTimestamp_ns = pTime.delta_timestamp_ns(
        m_oldTriggerTimestamp_ns, triggerTimestamp_ns,
        m_oldFrameCounter, frameCounter, stats_triggertime_wraps);

    if (deltaTriggerTimestamp_ns <= pTime.trigger_period()) {
      hits.subsequentTrigger(true);
    }
  }

  // Crucial step
  // Storing hit to appropriate buffer
  if (overThresholdFlag || (adc >= pADCThreshold)) {
    hits.store(strip, adc, chipTime);
  }

  if (newEvent) {
    DTRACE(DEB, "\neventNr  %d\n", m_eventNr);
    DTRACE(DEB, "fecID  %d\n", fecID);
  }

  // This is likely resolved. Candidate for removal?
  if ((frameCounter < m_oldFrameCounter)
      && !(m_oldFrameCounter > frameCounter + 1000000000)) {
    stats_fc_error++;
  }

  // m_timeStamp_ms is used for printing Trace info
  if (m_eventNr > 1) {
    m_timeStamp_ms = m_timeStamp_ms + deltaTriggerTimestamp_ns * 0.000001;
  }
  if (deltaTriggerTimestamp_ns > 0) {
    DTRACE(DEB, "\tTimestamp %.2f [ms]\n", m_timeStamp_ms);
    DTRACE(DEB, "\tTime since last trigger %.4f us (%.4f kHz)\n",
           deltaTriggerTimestamp_ns * 0.001,
           1000000 / deltaTriggerTimestamp_ns);
    DTRACE(DEB, "\tTriggerTimestamp %.2f [ns]\n", triggerTimestamp_ns);
  }
  if (m_oldFrameCounter != frameCounter || newEvent) {
    DTRACE(DEB, "\n\tFrameCounter %u\n", frameCounter);
  }
  if (m_oldVmmID != vmmID || newEvent) {
    DTRACE(DEB, "\tvmmID  %d\n", vmmID);
  }
  DTRACE(DEB, "\t\tchannel %d (chNo  %d) - overThresholdFlag %d\n",
         strip, chNo, overThresholdFlag);
  DTRACE(DEB, "\t\t\tbcid %d, tdc %d, adc %d\n", bcid, tdc, adc);
  DTRACE(DEB, "\t\t\tchipTime %.2f us\n", chipTime * 0.001);

  m_oldTriggerTimestamp_ns = triggerTimestamp_ns;
  m_oldFrameCounter = frameCounter;
  m_oldVmmID = vmmID;
}

void NMXHitSorter::AnalyzeClusters() {
  hits.sort_and_correct();
  callback_.ClusterByTime(hits.hits());

//  DTRACE(DEB, "%z cluster in x\n", m_tempClusterX.size());
//  DTRACE(DEB, "%z cluster in y\n", m_tempClusterY.size());
}




NMXClusterer::NMXClusterer(SRSTime time, size_t minClusterSize, double deltaTimeHits,
                           uint16_t deltaStripHits, double deltaTimeSpan) :
    pTime(time), pMinClusterSize(minClusterSize), pDeltaTimeHits(deltaTimeHits),
    pDeltaStripHits(deltaStripHits), pDeltaTimeSpan(deltaTimeSpan)
{
}

//====================================================================================================================
void NMXClusterer::ClusterByTime(const HitContainer &hits) {

  auto pre = clusters.size();

  HitContainer cluster;
  double maxDeltaTime = 0;
  size_t stripCount = 0;
  double time1 = 0, time2 = 0;
  uint16_t adc1 = 0;
  uint16_t strip1 = 0;

  for (const auto &itHits : hits) {
    time2 = time1;

    time1 = itHits.time;
    strip1 = itHits.strip;
    adc1 = itHits.adc;

    if (time1 - time2 <= pDeltaTimeHits && stripCount > 0
        && maxDeltaTime < (time1 - time2)) {
      maxDeltaTime = (time1 - time2);
    }

    if (time1 - time2 > pDeltaTimeHits && stripCount > 0) {
      ClusterByStrip(cluster/*, maxDeltaTime*/);
      cluster.clear();
      maxDeltaTime = 0;
    }
    cluster.emplace_back(Eventlet());
    auto &e = cluster[cluster.size()-1];
    e.strip = strip1;
    e.time = time1;
    e.adc = adc1;
    stripCount++;
  }

  if (stripCount > 0)
    ClusterByStrip(cluster);

  stats_cluster_count += (clusters.size() - pre);
}

//====================================================================================================================
void NMXClusterer::ClusterByStrip(HitContainer &hits) {
  PlaneNMX cluster;

  std::sort(begin(hits), end(hits),
            [](const Eventlet &e1, const Eventlet &e2) {
              return e1.strip < e2.strip;
            });

  for (auto &hit : hits) {
    // If empty cluster, just add and move on
    if (!cluster.entries.size()) {
      cluster.insert_eventlet(hit);
      continue;
    }

    // Stash cluster if strip gap to next hit is too large
    // filtering is done elsewhere
    auto strip_gap = std::abs(hit.strip - cluster.strip_end);
    if (strip_gap > (pDeltaStripHits + 1))
    {
      // Attempt to stash
      stash_cluster(cluster);

      // Reset and move on
      cluster = PlaneNMX();
    }

    // insert in either case
    cluster.insert_eventlet(hit);
  }

  // At the end of the clustering, check again if there is a last valid cluster
  if (cluster.entries.size() >= pMinClusterSize) {
    stash_cluster(cluster);
  }
}
//====================================================================================================================
void NMXClusterer::stash_cluster(PlaneNMX cluster) {

  // Some filtering can happen here
  if (cluster.entries.size() < pMinClusterSize)
    return;

  // pDeltaTimeSpan ?

  DTRACE(DEB, "******** VALID ********\n");
  ClusterNMX theCluster;
  theCluster.plane = cluster;
  clusters.emplace_back(std::move(theCluster));
}

bool NMXClusterer::ready() const
{
  return (clusters.size());
}