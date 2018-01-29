#include <algorithm>
#include <cmath>
#include <gdgem/dg_impl/RootFile.h>
#include <common/Trace.h>

#define UNUSED __attribute__((unused))

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

RootFile::RootFile(int bc, int tac, int acqWin, std::vector<int> xChips,
		std::vector<int> yChips, int adcThreshold, int minClusterSize,
		float deltaTimeHits, int deltaStripHits, float deltaTimeSpan,
		float deltaTimePlanes) :
		pBC(bc), pTAC(tac), pAcqWin(acqWin), pXChipIDs(xChips), pYChipIDs(
				yChips), pADCThreshold(adcThreshold), pMinClusterSize(
				minClusterSize), pDeltaTimeHits(deltaTimeHits), pDeltaStripHits(
				deltaStripHits), pDeltaTimeSpan(deltaTimeSpan), pDeltaTimePlanes(
				deltaTimePlanes)
{

	m_eventNr = 0;
}

RootFile::~RootFile()
{
}

//====================================================================================================================
int RootFile::AnalyzeHitData(int triggerTimestamp, int frameCounter, int fecID,
		int vmmID, int chNo, int bcid, int tdc, int adc, int overThresholdFlag)
{

	bool newEvent = false;
	double triggerTimestamp_ns = triggerTimestamp * 3.125;
	double deltaTriggerTimestamp_ns = 0;

	if (m_oldTriggerTimestamp_ns != triggerTimestamp_ns)
	{
		FillClusters();
		newEvent = true;
		m_subsequentTrigger = false;
		m_eventNr++;
	}

	if ((frameCounter < m_oldFrameCounter)
			&& !(m_oldFrameCounter > frameCounter + 1000000000))
	{
		DTRACE(DEB,
				"\n*********************************** SCRAMBLED eventNr  %d, old framecounter %d, new framecounter %d\n",
				m_eventNr, m_oldFrameCounter, frameCounter);
	}

	if (m_oldTriggerTimestamp_ns > triggerTimestamp_ns
			&& (m_oldFrameCounter <= frameCounter
					|| m_oldFrameCounter > frameCounter + 1000000000))
	{
		deltaTriggerTimestamp_ns = (13421772800 + triggerTimestamp_ns
				- m_oldTriggerTimestamp_ns);

	}
	else
	{
		deltaTriggerTimestamp_ns = (triggerTimestamp_ns
				- m_oldTriggerTimestamp_ns);
	}

	if (newEvent
			&& (deltaTriggerTimestamp_ns <= 1000 * 4096 * (1 / (double) pBC)))
	{
		m_subsequentTrigger = true;
	}

	if (m_eventNr > 1)
	{
		m_timeStamp_ms = m_timeStamp_ms + deltaTriggerTimestamp_ns * 0.000001;
	}

	int planeID = GetPlaneID(vmmID);

	// Plane 0: x
	// plane 1: y
	int x = -1;
	int y = -1;
	if (planeID == 0)
	{
		// Fix for entries with all zeros
		if (bcid == 0 && tdc == 0 && overThresholdFlag)
		{
			bcid = m_oldBcidX;
			tdc = m_oldTdcX;
		}
		m_oldBcidX = bcid;
		m_oldTdcX = tdc;
		x = GetChannel(pXChipIDs, vmmID, chNo);
	}
	else if (planeID == 1)
	{
		// Fix for entries with all zeros
		if (bcid == 0 && tdc == 0 && overThresholdFlag)
		{
			bcid = m_oldBcidY;
			tdc = m_oldTdcY;
		}
		m_oldBcidY = bcid;
		m_oldTdcY = tdc;
		y = GetChannel(pYChipIDs, vmmID, chNo);
	}

	// Calculate bcTime [us]
	double bcTime = bcid * (1 / (double) pBC);

	// TDC time: pTAC * tdc value (8 bit)/ramp length
	// [ns]

	// TDC has reduced resolution due to most significant bit problem of current
	// sources (like ADC)
	int tdcRebinned = (int) tdc / 8;
	tdc = tdcRebinned * 8;
	double tdcTime = pTAC * (double) tdc / 255;

	// Chip time: bcid plus tdc value
	// Talk Vinnie: HIT time  = BCIDx25 + ADC*125/255 [ns]
	double chipTime = bcTime * 1000 + tdcTime;

	AddHits(x, y, adc, bcid, chipTime, overThresholdFlag);

	if (newEvent)
	{
		DTRACE(DEB, "\neventNr  %d\n", m_eventNr);
		DTRACE(DEB, "fecID  %d\n", fecID);
	}

	if (deltaTriggerTimestamp_ns > 0)
	{
		DTRACE(DEB, "\tTimestamp %.2f [ms]\n", m_timeStamp_ms);
		DTRACE(DEB, "\tTime since last trigger %.4f us (%.4f kHz)\n",
				deltaTriggerTimestamp_ns * 0.001,
				1000000 / deltaTriggerTimestamp_ns);
		DTRACE(DEB, "\tTriggerTimestamp %.2f [ns]\n",
				triggerTimestamp_ns);
	}
	if (m_oldFrameCounter != frameCounter || newEvent)
	{
		DTRACE(DEB, "\n\tFrameCounter %u\n", frameCounter);
	}
	if (m_oldVmmID != vmmID || newEvent)
	{
		DTRACE(DEB, "\tvmmID  %d\n", vmmID);
	}
	if (planeID == 0)
	{
		DTRACE(DEB,
				"\t\tx-channel %d (chNo  %d) - overThresholdFlag %d\n", x, chNo,
				overThresholdFlag);
	}
	else if (planeID == 1)
	{
		DTRACE(DEB,
				"\t\ty-channel %d (chNo  %d) - overThresholdFlag %d\n", y, chNo,
				overThresholdFlag);
	}
	else
	{
		DTRACE(DEB, "\t\tPlane for vmmID %d not defined!\n", vmmID);
	}
	DTRACE(DEB, "\t\t\tbcid %d, tdc %d, adc %d\n", bcid, tdc, adc);
	DTRACE(DEB,
			"\t\t\tbcTime %.2f us, tdcTime %.2f ns, time %.2f us\n", bcTime,
			tdcTime, chipTime * 0.001);

	m_oldTriggerTimestamp_ns = triggerTimestamp_ns;
	m_oldFrameCounter = frameCounter;
	m_oldVmmID = vmmID;
	return 0;
}

//====================================================================================================================
void RootFile::AddHits(short x, short y, short adc, short bcid, float chipTime, bool overThresholdFlag)
{

	if (x > -1 && (adc >= pADCThreshold || overThresholdFlag))
	{

		if (bcid < pAcqWin * pBC / 40)
		{
			m_hitsX.insert(std::make_pair(chipTime, std::make_pair(x, adc)));

		}
		else
		{
			m_hitsOldX.insert(std::make_pair(chipTime, std::make_pair(x, adc)));
		}
	}
	if (y > -1 && (adc >= pADCThreshold || overThresholdFlag))
	{
		if (bcid < pAcqWin * pBC / 40)
		{
			m_hitsY.insert(std::make_pair(chipTime, std::make_pair(y, adc)));

		}
		else
		{
			m_hitsOldY.insert(std::make_pair(chipTime, std::make_pair(y, adc)));
		}
	}
}

//====================================================================================================================
int RootFile::ClusterByTime(std::multimap<float, std::pair<int, int>> &oldHits,
		float dTime, int dStrip, float dSpan, string coordinate)
{

	std::multimap<int, std::pair<float, int>> cluster;
	std::multimap<float, std::pair<int, int>>::iterator itOldHits =
			oldHits.begin();
	int clusterCount = 0;
	int stripCount = 0;
	double time1 = 0, time2 = 0;
	int adc1 = 0;
	int strip1 = 0;
	cluster.clear();
	for (; itOldHits != oldHits.end(); itOldHits++)
	{
		time2 = time1;
		time1 = itOldHits->first;
		strip1 = itOldHits->second.first;
		adc1 = itOldHits->second.second;
		if (time1 - time2 > dTime && stripCount > 0)
		{
			clusterCount += ClusterByStrip(cluster, dStrip, dSpan, coordinate);
			cluster.clear();
		}
		cluster.insert(std::make_pair(strip1, std::make_pair(time1, adc1)));
		stripCount++;
	}
	if (stripCount > 0)
	{
		clusterCount += ClusterByStrip(cluster, dStrip, dSpan, coordinate);
	}
	return clusterCount;
}

//====================================================================================================================
int RootFile::ClusterByStrip(
		std::multimap<int, std::pair<float, int>> &cluster, int dStrip,
		float dSpan, string coordinate)
{
	float startTime = 0;
	float largestTime = 0;
	float clusterPositionUTPC = -1;

	float centerOfGravity = -1;
	float centerOfTime = 0;
	int totalADC = 0;
	float time1 = 0;
	int adc1 = 0;
	int strip1 = 0, strip2 = 0;
	int stripCount = 0;
	int clusterCount = 0;

	std::multimap<int, std::pair<float, int>>::iterator itCluster =
			cluster.begin();
	for (; itCluster != cluster.end(); itCluster++)
	{
		strip2 = strip1;
		strip1 = itCluster->first;
		time1 = itCluster->second.first;
		adc1 = itCluster->second.second;
		// At beginning of cluster, set start time of cluster
		if (stripCount == 0)
		{
			startTime = time1;
			DTRACE(DEB, "\n%s cluster:\n", coordinate.c_str());
		}

		// Add members of a cluster, if it is either the beginning of a cluster,
		// or if strip gap and time span is correct
		if (stripCount == 0
				|| (abs(strip1 - strip2) > 0
						&& abs(strip1 - strip2) <= (dStrip + 1)
						&& time1 - startTime <= dSpan))
		{
			DTRACE(DEB, "\tstrip %d, time %f, adc %d:\n", strip1,
					time1, adc1);
			if (time1 > largestTime)
			{
				largestTime = time1;
				clusterPositionUTPC = strip1;
			}
			if (time1 < startTime)
			{
				startTime = time1;
			}
			centerOfGravity += strip1 * adc1;
			centerOfTime += time1 * adc1;
			totalADC += adc1;
			stripCount++;
		}
		// Stop clustering if gap between strips is too large or time span too long
		else if (abs(strip1 - strip2) > (dStrip + 1)
				|| largestTime - startTime > dSpan)
		{
			// Valid cluster
			if (stripCount >= pMinClusterSize)
			{
				centerOfGravity = (centerOfGravity / (float) totalADC);
				centerOfTime = (centerOfTime / (float) totalADC);
				AddClusters(centerOfGravity, clusterPositionUTPC, stripCount,
						totalADC, centerOfTime, largestTime, coordinate);
				clusterCount++;
				DTRACE(DEB, "******** VALID ********\n");

			}

			// Reset all parameters
			startTime = 0;
			largestTime = 0;
			clusterPositionUTPC = 0;
			stripCount = 0;
			centerOfGravity = 0;
			centerOfTime = 0;
			totalADC = 0;
			strip1 = 0;
		}
	}
	// At the end of the clustering, check again if there is a last valid cluster
	if (stripCount >= pMinClusterSize)
	{
		centerOfGravity = (centerOfGravity / (float) totalADC);
		centerOfTime = (centerOfTime / totalADC);
		AddClusters(centerOfGravity, clusterPositionUTPC, stripCount, totalADC,
				centerOfTime, largestTime, coordinate);
		clusterCount++;
		DTRACE(DEB, "******** VALID ********\n");
	}
	return clusterCount;
}

//====================================================================================================================
void RootFile::AddClusters(float clusterPosition, float clusterPositionUTPC,
		short clusterSize, int clusterADC, float clusterTime,
		float clusterTimeUTPC, string coordinate)
{

	cluster theCluster;
	theCluster.size = clusterSize;
	theCluster.adc = clusterADC;
	theCluster.time = clusterTime;
	theCluster.time_uTPC = clusterTimeUTPC;
	theCluster.position = clusterPosition;
	theCluster.position_uTPC = clusterPositionUTPC;
	theCluster.clusterXAndY = false;
	theCluster.clusterXAndY_uTPC = false;

	if (coordinate == "x" && clusterPosition > -1.0)
	{
		m_tempClusterX.push_back(theCluster);
	}
	if (coordinate == "y" && clusterPosition > -1.0)
	{
		m_tempClusterY.push_back(theCluster);
	}
}

//====================================================================================================================
void RootFile::MatchClustersXY(float dPlane)
{

	for (unsigned int nx = 0; nx < m_tempClusterX.size(); nx++)
	{
		float tx = m_tempClusterX[nx].time;
		float posx = m_tempClusterX[nx].position;

		float deltaT = 0;

		for (unsigned int ny = 0; ny < m_tempClusterY.size(); ny++)
		{
			float ty = m_tempClusterY[ny].time;
			float posy = m_tempClusterY[ny].position;

			deltaT = abs(ty - tx);

			if (deltaT <= dPlane && m_tempClusterY[ny].clusterXAndY == false
			&& m_tempClusterX[nx].clusterXAndY == false)
			{

				m_tempClusterX[nx].clusterXAndY = true;
				m_tempClusterY[ny].clusterXAndY = true;

				commonCluster theCommonCluster;
				theCommonCluster.sizeX = m_tempClusterX[nx].size;
				theCommonCluster.sizeY = m_tempClusterY[ny].size;
				theCommonCluster.adcX = m_tempClusterX[nx].adc;
				theCommonCluster.adcY = m_tempClusterY[ny].adc;
				theCommonCluster.positionX = m_tempClusterX[nx].position;
				theCommonCluster.positionY = m_tempClusterY[ny].position;
				theCommonCluster.timeX = m_tempClusterX[nx].time;
				theCommonCluster.timeY = m_tempClusterY[ny].time;
				m_clusterXY.push_back(theCommonCluster);

				DTRACE(DEB, "\ncommon cluster x/y (center of mass):");
				DTRACE(DEB, "\tpos x/pos y: %f/%f", posx, posy);
				DTRACE(DEB, "\ttime x/time y: : %f/%f", tx, ty);
				DTRACE(DEB, "\tadc x/adc y: %u/%u",
						theCommonCluster.adcX, theCommonCluster.adcY);
				DTRACE(DEB, "\tsize x/size y: %u/%u",
						theCommonCluster.sizeX, theCommonCluster.sizeY);

				break;
			}
		}
	}

	for (unsigned int nx = 0; nx < m_tempClusterX.size(); nx++)
	{
		float tx = m_tempClusterX[nx].time_uTPC;
		float posx = m_tempClusterX[nx].position_uTPC;

		float deltaT = 0;

		for (unsigned int ny = 0; ny < m_tempClusterY.size(); ny++)
		{
			float ty = m_tempClusterY[ny].time_uTPC;
			float posy = m_tempClusterY[ny].position_uTPC;

			deltaT = abs(ty - tx);

			if (deltaT <= dPlane
					&& m_tempClusterY[ny].clusterXAndY_uTPC == false
					&& m_tempClusterX[nx].clusterXAndY_uTPC == false)
			{

				m_tempClusterX[nx].clusterXAndY_uTPC = true;
				m_tempClusterY[ny].clusterXAndY_uTPC = true;

				commonCluster theCommonCluster_uTPC;
				theCommonCluster_uTPC.sizeX = m_tempClusterX[nx].size;
				theCommonCluster_uTPC.sizeY = m_tempClusterY[ny].size;
				theCommonCluster_uTPC.adcX = m_tempClusterX[nx].adc;
				theCommonCluster_uTPC.adcY = m_tempClusterY[ny].adc;
				theCommonCluster_uTPC.positionX = m_tempClusterX[nx].position;
				theCommonCluster_uTPC.positionY = m_tempClusterY[ny].position;
				theCommonCluster_uTPC.timeX = m_tempClusterX[nx].time;
				theCommonCluster_uTPC.timeY = m_tempClusterY[ny].time;
				m_clusterXY_uTPC.push_back(theCommonCluster_uTPC);

				DTRACE(DEB, "\ncommon cluster x/y (center of mass):");
				DTRACE(DEB, "\tpos x/pos y: %f/%f", posx, posy);
				DTRACE(DEB, "\ttime x/time y: : %f/%f", tx, ty);
				DTRACE(DEB, "\tadc x/adc y: %u/%u",
						theCommonCluster_uTPC.adcX, theCommonCluster_uTPC.adcY);
				DTRACE(DEB, "\tsize x/size y: %u/%u",
						theCommonCluster_uTPC.sizeX,
						theCommonCluster_uTPC.sizeY);

				break;
			}
		}
	}
}

//====================================================================================================================
void RootFile::FillClusters()
{
	CorrectTriggerData(m_hitsX, m_hitsOldX, pDeltaTimeHits);
	CorrectTriggerData(m_hitsY, m_hitsOldY, pDeltaTimeHits);
	int cntX = ClusterByTime(m_hitsOldX, pDeltaTimeHits,
			pDeltaStripHits, pDeltaTimeSpan, "x");
	int cntY = ClusterByTime(m_hitsOldY, pDeltaTimeHits,
			pDeltaStripHits, pDeltaTimeSpan, "y");

	DTRACE(DEB, "%d cluster in x\n", cntX);
	DTRACE(DEB, "%d cluster in y\n", cntY);

	MatchClustersXY(pDeltaTimePlanes);

	m_hitsOldX.clear();
	m_hitsOldY.clear();
	if (!m_hitsX.empty())
		m_hitsOldX = m_hitsX;
	if (!m_hitsY.empty())
		m_hitsOldY = m_hitsY;
	m_hitsX.clear();
	m_hitsY.clear();
	m_clusterX.insert(m_clusterX.end(), m_tempClusterX.begin(), m_tempClusterX.end());
	m_clusterY.insert(m_clusterY.end(), m_tempClusterY.begin(), m_tempClusterY.end());
	m_tempClusterX.clear();
	m_tempClusterY.clear();

}

//====================================================================================================================
void RootFile::CorrectTriggerData(
		std::multimap<float, std::pair<int, int>> &hits,
		std::multimap<float, std::pair<int, int>> &oldHits,
		float correctionTime)
{
	if (oldHits.size() > 0 && hits.size() > 0)
	{
		if (m_subsequentTrigger)
		{
			std::multimap<float, std::pair<int, int>>::iterator itBegin =
					hits.begin();
			std::multimap<float, std::pair<int, int>>::iterator itEnd =
					hits.end();
			std::multimap<float, std::pair<int, int>>::reverse_iterator itReverseEnd =
					oldHits.rbegin();
			float bcPeriod = 1000 * 4096 * (1 / (float) pBC);
			float timePrevious = itReverseEnd->first;
			float timeNext = itBegin->first + bcPeriod;
			float deltaTime = timeNext - timePrevious;

			if (deltaTime <= correctionTime)
			{
				std::multimap<float, std::pair<int, int>>::iterator it;
				std::multimap<float, std::pair<int, int>>::iterator itFind;

				for (it = itBegin; it != itEnd; ++it)
				{
					timePrevious = timeNext;
					timeNext = (*it).first + bcPeriod;
					deltaTime = timeNext - timePrevious;
					itFind = it;
					if (deltaTime > correctionTime)
					{
						break;
					}
					else
					{
						oldHits.insert(
								std::make_pair(timeNext,
										std::make_pair(it->second.first,
												it->second.second)));
					}
				}
				hits.erase(hits.begin(), itFind);
			}
		}
	}
}

//====================================================================================================================
int RootFile::GetPlaneID(int chipID)
{
	std::vector<int>::iterator it;

	it = find(pXChipIDs.begin(), pXChipIDs.end(), chipID);
	if (it != pXChipIDs.end())
	{
		return 0;
	}
	else
	{
		it = find(pYChipIDs.begin(), pYChipIDs.end(), chipID);
		if (it != pYChipIDs.end())
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
}

//====================================================================================================================
int RootFile::GetChannel(std::vector<int>& chipIDs, int chipID,
		int channelID)
{
	std::vector<int>::iterator it;

	it = find(chipIDs.begin(), chipIDs.end(), chipID);
	if (it != chipIDs.end())
	{
		int pos = it - chipIDs.begin();
		return channelID + pos * 64;
	}
	else
	{
		return -1;
	}
}
