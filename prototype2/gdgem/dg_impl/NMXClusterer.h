#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <future>   // async

#include <gdgem/vmm2srs/SRSMappings.h>
#include <gdgem/vmm2srs/SRSTime.h>

using std::string;

struct ClusterNMX {
	int size;
	int adc;
	float position;
	float position_uTPC;
	float time;
	float time_uTPC;
	bool clusterXAndY;
	bool clusterXAndY_uTPC;
	float maxDeltaTime;
	float maxDeltaStrip;
	float deltaSpan;

};

struct CommonClusterNMX {
	int sizeX;
	int sizeY;
	int adcX;
	int adcY;
	float positionX;
	float positionY;
	float timeX;
	float timeY;
	float deltaPlane;
	float maxDeltaTimeX;
	float maxDeltaTimeY;
	int maxDeltaStripX;
	int maxDeltaStripY;
};

using HitContainer = std::vector<std::tuple<float, int, int>>;
using ClusterContainer = std::vector<std::tuple<int, float, int>>;
using HitTuple = std::tuple<float, int, int>;
using ClusterTuple = std::tuple<int, float, int>;
using ClusterVector = std::vector<ClusterNMX>;

class NMXClusterer {
public:
	NMXClusterer(SRSTime time,
				 SRSMappings chips,
				 int acqWin,
				 int adcThreshold, int minClusterSize,
				 float deltaTimeHits, int deltaStripHits, float deltaTimeSpan,
				 float deltaTimePlanes);

	~NMXClusterer();

	// Analyzing and storing the hits
	bool AnalyzeHits(int triggerTimestamp, unsigned int frameCounter, int fecID,
			int vmmID, int chNo, int bcid, int tdc, int adc,
			int overThresholdFlag);
	void StoreX(uint16_t strip, short adc, short bcid, float chipTime);
    void StoreY(uint16_t strip, short adc, short bcid, float chipTime);

  // Analyzing and storing the clusters
	void AnalyzeClusters();
	void AsyncClustererX();
	void AsyncClustererY();


	int ClusterByTime(HitContainer& oldHits, float dTime, int dStrip,
				float dSpan, string coordinate);
	int ClusterByStrip(ClusterContainer &cluster, int dStrip, float dSpan,
				string coordinate, float maxDeltaTime);

	void StoreClusters(float clusterPosition, float clusterPositionUTPC,
				short clusterSize, int clusterADC, float clusterTime,
				float clusterTimeUTPC, string coordinate, float maxDeltaTime, int maxDeltaStrip, float deltaSpan);


	void MatchClustersXY(float dPlane);

	void CorrectTriggerData(HitContainer &hits, HitContainer &oldHits,
			float correctionTime);

	// Helper methods that map channels to strips
//	int GetPlaneID(int chipID);
//	int GetChannel(std::vector<int> &chipIDs, int chipID, int channelID);

	int getNumClustersX() {
		return m_clusterX.size();
	}
	;
	int getNumClustersY() {
		return m_clusterY.size();
	}
	;
	int getNumClustersXY() {
		return m_clusterXY.size();
	}
	;

	int getNumClustersXY_uTPC() {
		return m_clusterXY_uTPC.size();
	}
	;

	// Statistics counters
	uint64_t stats_fc_error { 0 };
	uint64_t stats_bcid_tdc_error { 0 };
	uint64_t stats_triggertime_wraps { 0 };

	const uint8_t planeID_X { 0 };
	const uint8_t planeID_Y { 1 };

private:
  	SRSTime pTime;
	SRSMappings pChips;

	int pAcqWin;
	int pADCThreshold;
	int pMinClusterSize;
	float pDeltaTimeHits;
	int pDeltaStripHits;
	float pDeltaTimeSpan;
	float pDeltaTimePlanes;

	int m_eventNr = 0;
	double m_oldTriggerTimestamp_ns = 0;
    unsigned int m_oldFrameCounter = 0;

    // For debug output only
    double m_timeStamp_ms = 0;
    int m_oldVmmID = 0;

    bool m_subsequentTrigger = false;

    // For all 0s correction
	int m_oldBcidX = 0;
	int m_oldTdcX = 0;
	int m_oldBcidY = 0;
	int m_oldTdcY = 0;

	HitContainer m_hitsOldX;
	HitContainer m_hitsOldY;
	HitContainer m_hitsX;
	HitContainer m_hitsY;

	std::vector<CommonClusterNMX> m_clusterXY;
	std::vector<CommonClusterNMX> m_clusterXY_uTPC;
	std::vector<ClusterNMX> m_tempClusterX;
	std::vector<ClusterNMX> m_tempClusterY;
	std::vector<ClusterNMX> m_clusterX;
	std::vector<ClusterNMX> m_clusterY;
};
