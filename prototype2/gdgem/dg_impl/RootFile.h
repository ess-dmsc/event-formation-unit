#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

typedef uint8_t UChar_t;

const int MESSAGE_EVENTS = 1000000000;
const int NFEC = 8;
const long max_hit = 100000000;


const int MAX_TIME_PARAMS = 5;
const double maxDeltaT[MAX_TIME_PARAMS] = {50,40,30,20,10};


const int MAX_STRIP_PARAMS = 5;
const int maxDeltaStrip[MAX_STRIP_PARAMS] = {5,4,3,2,1};

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

	//Filling the hits
	int AnalyzeHitData(unsigned int triggerTimestamp, unsigned int frameCounter,
			unsigned int fecID, unsigned int vmmID, unsigned int chNo, unsigned int bcid,unsigned int tdc, unsigned int adc,
			unsigned int overThresholdFlag);
	void AddHits(unsigned int eventNr, unsigned short fecID,
			unsigned short vmmID, double timeStamp, double triggerTimestamp,
			unsigned int frameCounter, UChar_t overThresholdFlag, unsigned short chNo,
			 short x,  short y, unsigned short adc, unsigned short tdc, unsigned short bcid,
			double chipTime);
	void FillHits();

	//Filling the clusters
	int ClusterByTime(//std::multimap<double, std::pair<int, unsigned int> >& hits,
				std::multimap<double, std::pair<int, unsigned int> >& oldHits,
				int dT, int dS, string coordinate);
	int ClusterByStrip(
				std::multimap<int, std::pair<double, unsigned int> > & cluster,
				int dS, string coordinate);
	void AddClusters(float clusterPosition, float clusterPositionUTPC,
				short clusterSize, unsigned int clusterADC, float clusterTime,
				string coordinate);
	void MatchClustersXY(int dT, int dS);
	void FillClusters();
	void CorrectTriggerData(std::multimap<double, std::pair<int, unsigned int> >& hits,
			std::multimap<double, std::pair<int, unsigned int> >& oldHits, int dT /*, string coordinate*/);

	//Helper methods that map channels to strips
	unsigned int GetPlaneID(unsigned int chipID);
	unsigned int GetChannelX(unsigned int chipID, unsigned int channelID,
			std::string readout);
	unsigned int GetChannelY(unsigned int chipID, unsigned int channelID,
			std::string readout);
	int MapChipChannelToReadout(unsigned int chNo, std::string readout);

	//Micromegas mapping methods
	int MMStripMappingHybrid1(unsigned int chNo);
	int MMStripMappingHybrid2(unsigned int chNo);
	int MMStripMappingHybrid3(unsigned int chNo);

	int getnCLinX() {return nCLinX;} // Readers added for google test
	int getnCLinY() {return nCLinY;}
	int getnCLinXY() {return nCLinXY;}
private:

	unsigned int firstFrameCounter = 0;
	unsigned int all_hits = 0;
	unsigned int m_nch = 0;
	unsigned int m_nchX = 0;
	unsigned int m_nchY = 0;

	unsigned int m_eventNr = 0;
	// unsigned int m_deltaBCID = 0;
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
	unsigned short* m_clusterSizeX = 0;
	unsigned short* m_clusterSizeY = 0;
	float * m_clusterX;
	float * m_clusterY = 0;
	float * m_clusterUTPCX = 0;
	float * m_clusterUTPCY = 0;
	unsigned int * m_clusterADCX = 0;
	unsigned int * m_clusterADCY = 0;
	double * m_clusterTimeX = 0;
	double * m_clusterTimeY = 0;



	std::multimap<double, float> clustersX;
	std::multimap<double, float> clustersY;

	std::multimap<double, std::pair<int, unsigned int> > hitsOldX;
	std::multimap<double, std::pair<int, unsigned int> > hitsOldY;

	std::multimap<double, std::pair<int, unsigned int> > hitsX;
	std::multimap<double, std::pair<int, unsigned int> > hitsY;

	std::multimap<double, std::pair<int, unsigned int> > hitsX_corrected;
	std::multimap<double, std::pair<int, unsigned int> > hitsY_corrected;

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
	unsigned int oldBcidX = 0;
	unsigned int oldBcidY = 0;
	unsigned int oldVmmID = -1;
	double chipTime = 0;
	double oldChipTime = 0;
	double bcTime = 0;
	double tdcTime = 0;

	unsigned int eventNr = 0;
	unsigned int planeID = -1;

	int x = -1;
	int y = -1;

	int nCLinX = 0;
	int nCLinY = 0;
	int nCLinXY = 0;
	int eventClusters = 0;

	int subsequentTrigger=0;
	std::map<int, double> lastTimeX;
	std::map<int, double> lastTimeY;

	std::map<int, double> lastBiggestTimeX;
	std::map<int, double> lastBiggestTimeY;
};
