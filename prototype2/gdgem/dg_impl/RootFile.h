#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

typedef uint8_t UChar_t;

const bool printDebugInfo = true;
const bool analyzeClusters = true;
const bool storeClusters = true;
const bool analyzeChannels = true;

const int MESSAGE_EVENTS = 1000;
const int NFEC = 8;
const long max_hit = 100000000;

const int MAX_DELTA_STRIP_HITS = 1;
const int deltaStripHits[MAX_DELTA_STRIP_HITS] =
{ 2 };
const int MAX_DELTA_TIME_HITS = 1;
const double deltaTimeHits[MAX_DELTA_TIME_HITS] =
{ 200 };

const int MAX_DELTA_TIME_SPAN = 1;
const double deltaTimeSpan[MAX_DELTA_TIME_SPAN] =
{ 500 };

const int MAX_DELTA_TIME_PLANES = 1;
const double deltaTimePlanes[MAX_DELTA_TIME_PLANES] =
{ 200 };

using namespace std;

class RootFile
{

public:
	//Setting up the root trees
	RootFile(int bc, int tac, int acqWin,
			std::vector<int> xChips, std::vector<int> yChips,
			std::string readout, bool viewEvent, int viewStart, int viewEnd,
			int threshold, int clusterSize);
	~RootFile();
	void InitRootFile();

	void DeleteHitTree();

	//Filling the hits
	int AnalyzeHitData(unsigned int triggerTimestamp, unsigned int frameCounter,
			unsigned int fecID, unsigned int vmmID, unsigned int chNo,
			unsigned int bcid, unsigned int tdc, unsigned int adc,
			unsigned int overThresholdFlag);
	void AddHits(unsigned short fecID,
			unsigned short vmmID, double timeStamp, double triggerTimestamp,
			unsigned int frameCounter, UChar_t overThresholdFlag,
			unsigned short chNo, short x, short y, unsigned short adc,
			unsigned short tdc, unsigned short bcid, double chipTime);


	//Filling the clusters
	int ClusterByTime(
			std::multimap<double, std::pair<int, unsigned int>>& oldHits,
			double dTime, double dStrip, double dSpan, string coordinate);
	int ClusterByStrip(
			std::multimap<int, std::pair<double, unsigned int>> & cluster,
			double dStrip, double dSpan, string coordinate);

	void AddClusters(float clusterPosition, float clusterPositionUTPC,
			short clusterSize, unsigned int clusterADC, float clusterTime,
			float clusterTimeUTPC, string coordinate);

	void MatchClustersXY(int a, int b, int c, int d);
	void FillClusters();
	void CorrectTriggerData(
			std::multimap<double, std::pair<int, unsigned int>>& hits,
			std::multimap<double, std::pair<int, unsigned int>>& oldHits,
			double correctionTime, string coordinate);

	//Helper methods that map channels to strips
	unsigned int GetPlaneID(unsigned int chipID);
	unsigned int GetChannelX(unsigned int chipID, unsigned int channelID,
			std::string readout);
	unsigned int GetChannelY(unsigned int chipID, unsigned int channelID,
			std::string readout);


	int getNumClustersX() {return m_nclX;};
	int getNumClustersY() {return m_nclY;};
	int getNumClustersXY() {return -1;};

private:



	unsigned int firstFrameCounter = 0;
	unsigned int all_hits = 0;
	unsigned int m_nch = 0;
	unsigned int m_nchX = 0;
	unsigned int m_nchY = 0;

	unsigned int m_eventNr = 0;
	unsigned int m_deltaBCID = 0;
	// fecID
	unsigned short m_fecID = 0;
	double m_timeStamp_ms = 0;
	double m_triggerTimestamp_ns = 0;

	unsigned int* m_frameCounter = 0;
	UChar_t * m_overThresholdFlag = 0;

	unsigned short * m_vmmID = 0;        // vmmID
	unsigned short * m_chNo = 0;       // Strip Number
	unsigned short * m_x = 0;
	unsigned short * m_y = 0;

	unsigned short * m_adc = 0;     //ADC value
	unsigned short * m_tdc = 0;    //TDC value
	unsigned short * m_bcid = 0;    //BCID value
	double * m_chipTime = 0;   //Composed time of BCID and TDC (1 ns resolution)

	unsigned long m_nclX = 0;
	unsigned long m_nclY = 0;

	unsigned short * m_clusterNumberX = 0;
	unsigned short * m_clusterNumberY = 0;
	unsigned short * m_clusterNumberUTPCX = 0;
	unsigned short * m_clusterNumberUTPCY = 0;
	unsigned short* m_clusterSizeX = 0;
	unsigned short* m_clusterSizeY = 0;
	float * m_clusterX;
	float * m_clusterY = 0;
	float * m_clusterUTPCX = 0;
	float * m_clusterUTPCY = 0;
	unsigned int * m_clusterADCX = 0;
	unsigned int * m_clusterADCY = 0;
	float * m_clusterTimeX = 0;
	float * m_clusterTimeY = 0;
	float * m_clusterTimeUTPCX = 0;
	float * m_clusterTimeUTPCY = 0;

	float * m_clusterDeltaStripX = 0;
	float * m_clusterDeltaStripY = 0;
	float* m_clusterDeltaTimeX = 0;
	float * m_clusterDeltaTimeY = 0;
	float* m_clusterDeltaSpanX = 0;
	float * m_clusterDeltaSpanY = 0;

	float m_commonX = 0;
	float m_commonY = 0;
	float m_commonSizeX = 0;
	float m_commonSizeY = 0;
	float m_commonADCX = 0;
	float m_commonADCY = 0;
	float m_commonTimeX = 0;
	float m_commonTimeY = 0;
	float m_deltaStripX = 0;
	float m_deltaStripY = 0;
	float m_deltaStrip = 0;
	float m_deltaTimeX = 0;
	float m_deltaTimeY = 0;
	float m_deltaTime = 0;
	float m_deltaSpanX = 0;
	float m_deltaSpanY = 0;
	float m_deltaSpan = 0;
	float m_deltaPlane = 0;

	std::multimap<double, std::pair<int, unsigned int> > hitsOldX;
	std::multimap<double, std::pair<int, unsigned int> > hitsOldY;

	std::multimap<double, std::pair<int, unsigned int> > hitsX;
	std::multimap<double, std::pair<int, unsigned int> > hitsY;

	int bcClock = 0;
	int tacSlope = 0;
	int acqWin = 0;
	std::vector<int> xChipIDs;
	std::vector<int> yChipIDs;
	std::string readoutType;

	bool fViewEvent = false;
	unsigned int fViewStart = 0;
	unsigned int fViewEnd = 0;
	int fThreshold = 0;
	int fMinClusterSize = 0;
	double triggerTimestamp_ns = 0;
	double oldTriggerTimestamp_ns = 0;
	double deltaTriggerTimestamp_ns = 0;
	double timeStamp_ms = 0;
	unsigned int oldFrameCounter = 0;

	unsigned int oldTdcX = 0;
	unsigned int oldTdcY = 0;
	unsigned int bcid = 0;
	unsigned int oldBcidX = 0;
	unsigned int oldBcidY = 0;
	unsigned int oldVmmID = -1;
	double chipTime = 0;
	double oldChipTime = 0;
	double bcTime = 0;
	double tdcTime = 0;
	//unsigned int eventNr = 0;
	unsigned int planeID = -1;
/*

	unsigned int fecID = 0;
	unsigned int vmmID = 0;

	unsigned int chNo = 0;
*/
	int x = -1;
	int y = -1;


	int subsequentTrigger = 0;

	int m_nx = 0;
	int m_ny = 0;

};
