#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

using namespace std;

struct Cluster
{
	unsigned int size;
	unsigned int adc;
	float position;
	float position_uTPC;
	float time;
	float time_uTPC;
	bool clusterXAndY;
	bool clusterXAndY_uTPC;
};

struct CommonCluster
{
	unsigned int sizeX;
	unsigned int sizeY;
	unsigned int adcX;
	unsigned int adcY;
	float positionX;
	float positionY;
	float timeX;
	float timeY;
};

class NMXClusterer
{
public:
	NMXClusterer(int bc, int tac, int acqWin, std::vector<int> xChips,
			std::vector<int> yChips, int adcThreshold, int minClusterSize,
			float deltaTimeHits, int deltaStripHits, float deltaTimeSpan,
			float deltaTimePlanes);

	~NMXClusterer();


// Analyzing and storing the hits
	int AnalyzeHits(int triggerTimestamp, unsigned int frameCounter, int fecID,
			int vmmID, int chNo, int bcid, int tdc, int adc,
			int overThresholdFlag);
	void StoreHits(short x, short y, short adc, short bcid, float chipTime, bool overThresholdFlag);

// Analyzing and storing the clusters
	void AnalyzeClusters();
	int ClusterByTime(std::multimap<float, std::pair<int, int>>&oldHits,
			float dTime, int dStrip, float dSpan, string coordinate);
	int ClusterByStrip(std::multimap<int, std::pair<float, int>> &cluster,
			int dStrip, float dSpan, string coordinate);

	void StoreClusters(float clusterPosition, float clusterPositionUTPC,
			short clusterSize, int clusterADC, float clusterTime,
			float clusterTimeUTPC, string coordinate);

	void MatchClustersXY(float dPlane);
	
	void CorrectTriggerData(std::multimap<float, std::pair<int, int>>&hits,
			std::multimap<float, std::pair<int, int>>&oldHits,
			float correctionTime);

// Helper methods that map channels to strips
	int GetPlaneID(int chipID);
	int GetChannel(std::vector<int>& chipIDs, int chipID, int channelID);

	int getNumClustersX()
	{
		return m_clusterX.size();
	}
	;
	int getNumClustersY()
	{
		return m_clusterY.size();
	}
	;
	int getNumClustersXY()
	{
		return m_clusterXY.size();
	}
	;

	int getNumClustersXY_uTPC()
	{
		return m_clusterXY_uTPC.size();
	}
	;

private:
	int pBC;
	int pTAC;
	int pAcqWin;
	std::vector<int> pXChipIDs;
	std::vector<int> pYChipIDs;

	int pADCThreshold;
	int pMinClusterSize;
	float pDeltaTimeHits;
	int pDeltaStripHits;
	float pDeltaTimeSpan;
	float pDeltaTimePlanes;

	int m_eventNr = 0;
	double m_oldTriggerTimestamp_ns = 0;
	bool m_subsequentTrigger = false;
	int m_oldVmmID = 0;
	unsigned int m_oldFrameCounter = 0;
	double m_timeStamp_ms = 0;
	int m_oldBcidX = 0;
	int m_oldTdcX = 0;
	int m_oldBcidY = 0;
	int m_oldTdcY = 0;

	std::multimap<float, std::pair<int, int> > m_hitsOldX;
	std::multimap<float, std::pair<int, int> > m_hitsOldY;

	std::multimap<float, std::pair<int, int> > m_hitsX;
	std::multimap<float, std::pair<int, int> > m_hitsY;

	std::vector<CommonCluster> m_clusterXY;
	std::vector<CommonCluster> m_clusterXY_uTPC;
	std::vector<Cluster> m_tempClusterX;
	std::vector<Cluster> m_tempClusterY;
	std::vector<Cluster> m_clusterX;
	std::vector<Cluster> m_clusterY;

};
