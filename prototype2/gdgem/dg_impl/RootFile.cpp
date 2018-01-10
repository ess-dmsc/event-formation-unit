#include <time.h>
#include <cstdlib>
#include <cmath>
#include "RootFile.h"

RootFile::RootFile(int bc, int tac, int acqWin,
		std::vector<int> xChips, std::vector<int> yChips, std::string readout,
		bool viewEvent, int viewStart, int viewEnd, int threshold,
		int clusterSize) :
		bcClock(bc), tacSlope(tac), acqWin(acqWin), xChipIDs(xChips), yChipIDs(
				yChips), readoutType(readout), fViewEvent(viewEvent), fViewStart(
				viewStart), fViewEnd(viewEnd), fThreshold(threshold), fMinClusterSize(
				clusterSize)
{
	InitRootFile();
}

RootFile::~RootFile()
{
	std::cout << "timeStamp_ms " << timeStamp_ms << std::endl;
	std::cout << "Total number of trigger periods/events " << m_eventNr << std::endl;

	std::cout << "Events with clusters " << eventClusters << std::endl;
	std::cout << "Total clusters in X " << nCLinX << std::endl;
	std::cout << "Total clusters in Y " << nCLinY << std::endl;
	std::cout << "Total clusters in X and Y " << nCLinXY << std::endl;
}

//====================================================================================================================
void RootFile::InitRootFile()
{
	m_eventNr = 0;
	m_nch = 0;
	m_nchX = 0;
	m_nchY = 0;

	m_nclX = 0;
	m_nclY = 0;

	m_frameCounter = new unsigned int[max_hit];
	m_vmmID = new unsigned short[max_hit];
	m_overThresholdFlag = new UChar_t[max_hit];
	m_chNo = new unsigned short[max_hit];
	m_x = new unsigned short[max_hit];
	m_y = new unsigned short[max_hit];
	m_adc = new unsigned short[max_hit];
	m_tdc = new unsigned short[max_hit];
	m_bcid = new unsigned short[max_hit];
	m_chipTime = new double[max_hit];
	m_clusterSizeX = new unsigned short[max_hit];
	m_clusterSizeY = new unsigned short[max_hit];
	m_clusterNumberX = new unsigned short[max_hit];
	m_clusterNumberY = new unsigned short[max_hit];
	m_clusterX = new float[max_hit];
	m_clusterY = new float[max_hit];
	m_clusterUTPCX = new float[max_hit];
	m_clusterUTPCY = new float[max_hit];
	m_clusterADCX = new unsigned int[max_hit];
	m_clusterTimeX = new double[max_hit];
	m_clusterADCY = new unsigned int[max_hit];
	m_clusterTimeY = new double[max_hit];
}

//====================================================================================================================
int RootFile::AnalyzeHitData(unsigned int triggerTimestamp,
		unsigned int frameCounter, unsigned int fecID, unsigned int vmmID,
		unsigned int chNo, unsigned int bcid, unsigned int tdc,
		unsigned int adc, unsigned int overThresholdFlag)
{
	if (eventNr == 0) {
		firstFrameCounter = frameCounter;
	}
	int newEvent = 0;
	triggerTimestamp_ns = triggerTimestamp * 3.125;

	if (oldTriggerTimestamp_ns != triggerTimestamp_ns) {
		eventNr++;
		newEvent = 1;
		all_hits += m_nch;
		if (eventNr % MESSAGE_EVENTS == 0) {
			std::cout << "Event: " << eventNr << "  -  total hits: " << all_hits
					<< std::endl;
		}

		FillHits();
		FillClusters();
		subsequentTrigger = 0;
	}

	if (oldVmmID != vmmID || newEvent) {
		oldChipTime = 0;
	}

	if ((frameCounter < oldFrameCounter) && !(oldFrameCounter > frameCounter + 1000000000)) {
		std::cout << "*********************************** SCRAMBLED " << eventNr
				<< " " << oldFrameCounter << " " << frameCounter << std::endl;
		//return -1;
	}

	if (oldTriggerTimestamp_ns > triggerTimestamp_ns
			&& (oldFrameCounter < frameCounter
					|| oldFrameCounter > frameCounter + 1000000000)) {
		deltaTriggerTimestamp_ns = (13421772800 + triggerTimestamp_ns
				- oldTriggerTimestamp_ns);
	} else {
		deltaTriggerTimestamp_ns = (triggerTimestamp_ns - oldTriggerTimestamp_ns);
	}

	if (newEvent && (deltaTriggerTimestamp_ns <= 1000 * 4096 * (1 / (double) bcClock))) {
		subsequentTrigger = 1;
	}

	if (eventNr > 1) {
		timeStamp_ms = timeStamp_ms + deltaTriggerTimestamp_ns * 0.000001;
	}

	planeID = GetPlaneID(vmmID);
	// Plane 0: x
	// plane 1: y
	if (planeID == 0) {
		//Fix for entries with all zeros
		if (bcid == 0 && tdc == 0 && overThresholdFlag) {
			bcid = oldBcidX;
			tdc = oldTdcX;
		}
		oldBcidX = bcid;
		oldTdcX = tdc;
		//lastTimeX[vmmID] = bcTime;

		x = GetChannelX(vmmID, chNo, readoutType);
		y = -1;
	}
	else if (planeID == 1) {
		//Fix for entries with all zeros
		if (bcid == 0 && tdc == 0 && overThresholdFlag) {
			bcid = oldBcidY;
			tdc = oldTdcY;
		}
		oldBcidY = bcid;
		oldTdcY = tdc;
		y = GetChannelY(vmmID, chNo, readoutType);
		x = -1;
	} else {
		x = -1;
		y = -1;
	}
	//Calculate bcTime [us]
	bcTime = bcid * (1 / (double) bcClock);
	//TDC time: tacSlope * tdc value (8 bit) * ramp length
	// [ns]
	tdcTime = tacSlope * (double) tdc / 255;
	//Chip time: bcid plus tdc value
	//Talk Vinnie: HIT time  = BCIDx25 + ADC*125/255 [ns]

	chipTime = bcTime * 1000 + tdcTime;

	AddHits(eventNr, fecID, vmmID, timeStamp_ms, triggerTimestamp_ns,
			frameCounter, overThresholdFlag, chNo, x, y, adc, tdc, bcid, chipTime);

	if (fViewEvent) {
		if (fViewEnd < eventNr && fViewEnd != 0) {
			return -1;
		}

		if ((fViewStart <= eventNr && fViewEnd >= eventNr)) {
			if (newEvent) {
				printf("\neventNr  %d\n", eventNr);
				printf("fecID  %d\n", fecID);
			}

			if (deltaTriggerTimestamp_ns > 0) {
				printf("\tTimestamp %.2f [ms]\n", timeStamp_ms);
				printf("\tTime since last trigger %.4f us (%.4f kHz)\n",
						deltaTriggerTimestamp_ns * 0.001,
						1000000 / deltaTriggerTimestamp_ns);
				printf("\tTriggerTimestamp %.2f [ns]\n", triggerTimestamp_ns);
			}

			if (oldFrameCounter != frameCounter || newEvent) {
				printf("\n\tFrameCounter %u\n", frameCounter);
			}

			if (oldVmmID != vmmID || newEvent) {
				printf("\tvmmID  %d\n", vmmID);
			}

			if (planeID == 0) {
				printf("\t\tx-channel %d (chNo  %d) - overThresholdFlag %d\n",
						x, chNo, overThresholdFlag);
			} else if (planeID == 1) {
				printf("\t\ty-channel %d (chNo  %d) - overThresholdFlag %d\n",
						y, chNo, overThresholdFlag);
			} else {
				printf("\t\tPlane for vmmID %d not defined!\n", vmmID);
			}

			printf("\t\t\tbcid %d, tdc %d, adc %d\n", bcid, tdc, adc);
			printf("\t\t\tbcTime %.2f us, tdcTime %.2f ns, time %.2f us\n",
					bcTime, tdcTime, chipTime * 0.001);
		}
	}

	oldTriggerTimestamp_ns = triggerTimestamp_ns;
	oldChipTime = chipTime;
	oldFrameCounter = frameCounter;
	oldVmmID = vmmID;
	return 0;
}

//====================================================================================================================
void RootFile::AddHits(unsigned int eventNr, unsigned short fecID,
		unsigned short vmmID, double timeStamp, double triggerTimestamp,
		unsigned int frameCounter, UChar_t overThresholdFlag,
		unsigned short chNo, short x, short y, unsigned short adc,
		unsigned short tdc, unsigned short bcid, double chipTime)
{

	if (m_nch < max_hit) {

		m_eventNr = eventNr;
		m_fecID = fecID;
		m_triggerTimestamp_ns = triggerTimestamp;
		m_timeStamp_ms = timeStamp;

		m_frameCounter[m_nch] = frameCounter;
		m_overThresholdFlag[m_nch] = overThresholdFlag;
		m_vmmID[m_nch] = vmmID;
		m_chNo[m_nch] = chNo;
		if (x > -1) {
			m_x[m_nchX] = x;
			m_nchX++;
		}
		if (y > -1) {
			m_y[m_nchY] = y;
			m_nchY++;
		}

		m_adc[m_nch] = adc;
		m_tdc[m_nch] = tdc;
		m_bcid[m_nch] = bcid;
		m_chipTime[m_nch] = chipTime;

		m_nch++;
		double bcPeriod = 1000 * 4096 * (1 / (double) bcClock);
		if (x > -1 && (adc >= fThreshold || overThresholdFlag)) {
			if (bcid < acqWin * bcClock / 40) {
				hitsX.insert(std::make_pair(chipTime, std::make_pair(x, adc)));
				hitsX_corrected.insert(
						std::make_pair(chipTime + bcPeriod,
								std::make_pair(x, adc)));
			} else {
				hitsOldX.insert(
						std::make_pair(chipTime, std::make_pair(x, adc)));

			}
		}
		if (y > -1 && (adc >= fThreshold || overThresholdFlag)) {
			if (bcid < acqWin * bcClock / 40) {
				hitsY.insert(std::make_pair(chipTime, std::make_pair(y, adc)));
				hitsY_corrected.insert(
						std::make_pair(chipTime + bcPeriod,
								std::make_pair(y, adc)));
			} else {
				hitsOldY.insert(
						std::make_pair(chipTime, std::make_pair(y, adc)));
			}
		}
	} else {
		std::cout << "ERROR! More than " << max_hit << " channels hit!" << std::endl;
	}
}

//====================================================================================================================
void RootFile::FillHits()
{
	m_nch = 0;
	m_nchX = 0;
	m_nchY = 0;
}

//====================================================================================================================
int RootFile::ClusterByTime(
		//std::multimap<double, std::pair<int, unsigned int>>& hits,
		std::multimap<double, std::pair<int, unsigned int>>& oldHits, int dT,
		int dS, string coordinate)
{
	int clusterCount = 0;

	std::multimap<int, std::pair<double, unsigned int> > cluster;

	std::multimap<double, std::pair<int, unsigned int>>::iterator itOldHits = oldHits.begin();

	int stripCount = 0;
	double time1 = 0, time2 = 0;
	unsigned int adc1 = 0;
	int strip1 = 0;
	cluster.clear();
	for (; itOldHits != oldHits.end(); itOldHits++) {
		time2 = time1;
		time1 = itOldHits->first;
		strip1 = itOldHits->second.first;
		adc1 = itOldHits->second.second;

		if ((time1 - time2 > maxDeltaT[dT]) && stripCount > 0) {
			clusterCount += ClusterByStrip(cluster, maxDeltaStrip[dS],
					coordinate);
			cluster.clear();
		}
		cluster.insert(std::make_pair(strip1, std::make_pair(time1, adc1)));
		stripCount++;
	}

	clusterCount += ClusterByStrip(cluster, dS, coordinate);

	return clusterCount;
}

//====================================================================================================================
int RootFile::ClusterByStrip(
		std::multimap<int, std::pair<double, unsigned int>> & cluster, int dS,
		string coordinate)
{
	std::multimap<int, std::pair<double, unsigned int> >::iterator itCluster = cluster.begin();
	double lastTime = 0;
	double lastTimeStrip = 0;
	double centerOfGravity = 0;
	double centerOfTime = 0;
	unsigned int totalADC = 0;
	double time1 = 0;
	unsigned int adc1 = 0;
	int strip1 = 0, strip2 = 0;
	int stripCount = 0;
	int clusterCount = 0;

	for (; itCluster != cluster.end(); itCluster++) {
		adc1 = itCluster->second.second;
		strip2 = strip1;
		strip1 = itCluster->first;
		time1 = itCluster->second.first;

		if (stripCount == 0 || (abs(strip1 - strip2) > 0
						&& abs(strip1 - strip2) <= maxDeltaStrip[dS])) {
			if (time1 > lastTime) {
				lastTime = time1;
				lastTimeStrip = strip1;
			}
			centerOfGravity += strip1 * adc1;
			centerOfTime += time1 * adc1;
			totalADC += adc1;
			stripCount++;
		} else if (abs(strip1 - strip2) > maxDeltaStrip[dS]) {
			centerOfGravity = (centerOfGravity / (double) totalADC);
			centerOfTime = (centerOfTime / totalADC);
			if (stripCount >= fMinClusterSize) {
				AddClusters(centerOfGravity, lastTimeStrip, stripCount,
						totalADC, lastTime, coordinate);
				clusterCount++;
			}

			lastTime = 0;
			lastTimeStrip = 0;
			stripCount = 0;
			centerOfGravity = 0;
			centerOfTime = 0;
			totalADC = 0;
			strip1 = 0;
		}
	}

	if (stripCount > 0) {
		centerOfGravity = (centerOfGravity / (double) totalADC);
		centerOfTime = (centerOfTime / totalADC);
		if (stripCount >= fMinClusterSize) {
			AddClusters(centerOfGravity, lastTimeStrip, stripCount, totalADC,
					lastTime, coordinate);
			clusterCount++;
		}
	}
	return clusterCount;
}

//====================================================================================================================
void RootFile::AddClusters(float clusterPosition, float clusterPositionUTPC,
		short clusterSize, unsigned int clusterADC, float clusterTime,
		string coordinate)
{
	if (m_nclX < max_hit || m_nclY < max_hit) {
		if (coordinate == "x" && clusterPosition > -1.0) {
			m_clusterX[m_nclX] = clusterPosition;
			m_clusterUTPCX[m_nclX] = clusterPositionUTPC;
			m_clusterSizeX[m_nclX] = clusterSize;
			m_clusterADCX[m_nclX] = clusterADC;
			m_clusterTimeX[m_nclX] = clusterTime;
			m_clusterNumberX[m_nclX] = 0;
			clustersX.insert(std::make_pair(clusterTime, clusterPosition));
			m_nclX++;
		}
		if (coordinate == "y" && clusterPosition > -1.0) {
			m_clusterY[m_nclY] = clusterPosition;
			m_clusterUTPCY[m_nclY] = clusterPositionUTPC;
			m_clusterSizeY[m_nclY] = clusterSize;
			m_clusterADCY[m_nclY] = clusterADC;
			m_clusterTimeY[m_nclY] = clusterTime;
			m_clusterNumberY[m_nclY] = 0;
			clustersY.insert(std::make_pair(clusterTime, clusterPosition));
			m_nclY++;
		}
	} else {
		std::cout << "ERROR! More than " << max_hit << " clusters produced!"
				<< std::endl;
	}
}

//====================================================================================================================
void RootFile::MatchClustersXY(int dT, int dS)
{
	if (!fViewEvent || (fViewEvent && fViewStart <= m_eventNr && fViewEnd >= m_eventNr)) {
		std::multimap<double, float>::iterator itClusterX = clustersX.begin();

		int nx = 0;

		if (clustersX.size() > 0) {
			nCLinX += clustersX.size();
		}
		if (clustersY.size() > 0) {
			nCLinY += clustersY.size();
		}
		if (clustersX.size() > 0 || clustersY.size() > 0) {
			eventClusters++;
		}
		for (; itClusterX != clustersX.end(); itClusterX++) {
			double tx = itClusterX->first;
			double posx = itClusterX->second;

			int ny = 0;
			double oldDeltaT = 999999999999;
			double deltaT = 0;
			std::multimap<double, float>::iterator itClusterY = clustersY.begin();
			for (; itClusterY != clustersY.end(); itClusterY++) {
				double ty = itClusterY->first;
				double posy = itClusterY->second;
				deltaT = std::abs(ty - tx);
				if (deltaT
						<= (maxDeltaT[dT]
								* fMinClusterSize)
						&& m_clusterNumberY[ny] == 0
						&& m_clusterNumberX[nx] == 0) {

					m_clusterNumberY[ny] = 1;
					m_clusterNumberX[nx] = 1;
					nCLinXY++;

					if (fViewEvent) {
						std::cout
								<< "************************************************************"
								<< std::endl;
						std::cout << "max delta T: " << maxDeltaT[dT]
								<< ", max delta strip: " << maxDeltaStrip[dS]
								<< std::endl;
						std::cout << "EventNr " << m_eventNr
								<< " cluster (delta t, tx, ty, posx, posy, sizex, sizey) "
								<< deltaT << "," << tx << ", " << ty << ","
								<< posx << "," << posy << ","
								<< m_clusterSizeX[nx] << ","
								<< m_clusterSizeY[ny] << std::endl;
						std::cout
								<< "************************************************************"
								<< std::endl;
					}
					break;
				}
				oldDeltaT = deltaT;
				ny++;

			}
			nx++;
		}
	}

	clustersX.clear();
	clustersY.clear();
	m_nclX = 0;
	m_nclY = 0;
}

//====================================================================================================================
void RootFile::FillClusters()
{

	if (!fViewEvent || (fViewStart <= eventNr && fViewEnd >= eventNr)) {
		CorrectTriggerData(hitsX_corrected, hitsOldX, 0 /*, "x" */);
		CorrectTriggerData(hitsY_corrected, hitsOldY, 0 /*, "y" */);

		for (int a = 0; a < MAX_TIME_PARAMS; a++) {
			for (int b = 0; b < MAX_STRIP_PARAMS; b++) {
				int clusterCount = ClusterByTime(/* hitsX_corrected, */ hitsOldX, a, b, "x");
				clusterCount = ClusterByTime(/* hitsY_corrected, */ hitsOldY, a, b, "y");
				MatchClustersXY(a, b);
			}
		}
	}
	hitsOldX.clear();
	hitsOldY.clear();
	hitsOldX = hitsX;
	hitsOldY = hitsY;
	hitsX.clear();
	hitsY.clear();
	hitsX_corrected.clear();
	hitsY_corrected.clear();
}


//====================================================================================================================
void RootFile::CorrectTriggerData(std::multimap<double, std::pair<int, unsigned int>>& hits,
		std::multimap<double, std::pair<int, unsigned int>>& oldHits, int dT /*, string coordinate*/)
{
	if (oldHits.size() > 0 && hits.size() > 0) {

		if (subsequentTrigger) {
			std::multimap<double, std::pair<int, unsigned int> >::iterator itBegin =
					hits.begin();
			std::multimap<double, std::pair<int, unsigned int> >::iterator itEnd =
					hits.end();
			std::multimap<double, std::pair<int, unsigned int> >::reverse_iterator itReverseEnd =
					oldHits.rbegin();

			double timePrevious = itReverseEnd->first;
			double timeNext = itBegin->first;
			double deltaTime = timeNext - timePrevious;

			if (deltaTime <= maxDeltaT[dT]) {
				std::multimap<double, std::pair<int, unsigned int>>::iterator it;
				std::multimap<double, std::pair<int, unsigned int>>::iterator itFind;

				for (it = itBegin; it != itEnd; ++it) {
					timePrevious = timeNext;
					timeNext = (*it).first;
					deltaTime = timeNext - timePrevious;
					itFind = it;
					if (deltaTime > maxDeltaT[dT]) {
						break;
					}
				}
				oldHits.insert(hits.begin(), itFind);
				hits.erase(hits.begin(), itFind);
			}
		}
	}
}

//====================================================================================================================
unsigned int RootFile::GetPlaneID(unsigned int chipID)
{
	std::vector<int>::iterator it;

	it = find(xChipIDs.begin(), xChipIDs.end(), chipID);
	if (it != xChipIDs.end()) {
		return 0;
	} else {
		it = find(yChipIDs.begin(), yChipIDs.end(), chipID);
		if (it != yChipIDs.end()) {
			return 1;
		} else {
			return -1;
		}
	}
}

//====================================================================================================================
unsigned int RootFile::GetChannelX(unsigned int chipID, unsigned int channelID,
		std::string readout)
{
	std::vector<int>::iterator it;

	it = find(xChipIDs.begin(), xChipIDs.end(), chipID);
	if (it != xChipIDs.end()) {
		int pos = it - xChipIDs.begin();
		return MapChipChannelToReadout(channelID + pos * 64, readout);
	} else {
		return -1;
	}
}

//====================================================================================================================
unsigned int RootFile::GetChannelY(unsigned int chipID, unsigned int channelID,
		std::string readout)
{
	std::vector<int>::iterator it;

	it = find(yChipIDs.begin(), yChipIDs.end(), chipID);
	if (it != yChipIDs.end()) {
		int pos = it - yChipIDs.begin();
		return MapChipChannelToReadout(channelID + pos * 64, readout);;
	} else {
		return -1;
	}
}

//====================================================================================================================
int RootFile::MapChipChannelToReadout(unsigned int chNo, std::string readout)
{
	if (readout == "MM1" || readout == "mm1") {
		return MMStripMappingHybrid1(chNo);
	}
	if (readout == "MM2" || readout == "mm2") {
		return MMStripMappingHybrid2(chNo);
	}
	if (readout == "MM3" || readout == "mm3") {
		return MMStripMappingHybrid3(chNo);
	}
	if (readout == "GEM" || readout == "gem") {
		return (chNo);
	}
	return (chNo);
}

//====================================================================================================================
int RootFile::MMStripMappingHybrid1(unsigned int chNo)
{
	if ((chNo % 2) == 1) {
		chNo = ((chNo - 1) / 2) + 32;
	} else {
		chNo = (chNo / 2);
		if (chNo < 32)
			chNo = 31 - chNo;
		else if (chNo > 37)
			chNo = 159 - chNo;
		else
			chNo += 90;
	}
	return chNo;
}

//=====================================================
int RootFile::MMStripMappingHybrid2(unsigned int chNo)
{
	if ((chNo % 2) == 1) {
		chNo = ((chNo - 1) / 2) + 27;
	} else {
		chNo = (chNo / 2);
		if (chNo < 27)
			chNo = 26 - chNo;
		else if (chNo > 38)
			chNo = 154 - chNo;
		else
			chNo += 89;
	}
	return chNo;
}

//====================================================================================================================
int RootFile::MMStripMappingHybrid3(unsigned int chNo)
{
	if ((chNo % 2) == 1) {
		chNo = ((chNo - 1) / 2) + 26;
	} else {
		chNo = (chNo / 2);
		if (chNo < 26)
			chNo = 25 - chNo;
		else if (chNo > 31)
			chNo = 153 - chNo;
		else
			chNo += 96;
	}
	return chNo;
}
