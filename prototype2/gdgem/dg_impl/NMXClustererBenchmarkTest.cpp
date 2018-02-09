/** Copyright (C) 2018 European Spallation Source ERIC */

#include <benchmark/benchmark.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/dg_impl/NMXClusterer.h>
#include <gdgem/dg_impl/TestData.h>
#include <test/TestBase.h>



static void Doit(benchmark::State &state)
{
	std::vector<int> pXChips {0, 1, 6, 7};
	std::vector<int> pYChips {10, 11, 14, 15};
	int pTAC = 60;
	int pBC = 20;
	int pAcqWin = 4000;
	int pADCThreshold=0;
  int pMinClusterSize = 3;
	//Maximum time difference between strips in time sorted cluster (x or y)
  float pDeltaTimeHits = 200;
	//Number of missing strips in strip sorted cluster (x or y)
	int pDeltaStripHits = 2;
	//Maximum time span for total cluster (x or y)
	float pDeltaTimeSpan = 500;
	//Maximum cluster time difference between matching clusters in x and y
	//Cluster time is either calculated with center-of-mass or uTPC method
	float pDeltaTimePlanes = 200;

  uint32_t items = 0;



 NMXClusterer nmxdata(pBC, pTAC, pAcqWin, pXChips, pYChips, pADCThreshold, pMinClusterSize, pDeltaTimeHits, pDeltaStripHits,pDeltaTimeSpan,pDeltaTimePlanes);
  for (auto _ : state) {
  	for (auto hit : Run16_line_110168_110323) { // replace with UDP receive()
		  int result = nmxdata.AnalyzeHits(hit.srs_timestamp, hit.framecounter, hit.fec, hit.chip_id, hit.channel, hit.bcid, hit.tdc, hit.adc, hit.overthreshold);

		  if (result == -1) {
			  printf("result == -1\n");
			  break;
		  }
    }
    items += 156; // 156 hits in the Run16_line_110168_110323 dataset
  }
  // state.SetComplexityN(state.range(0));
  //state.SetBytesProcessed(state.iterations() * state.range(0));
  state.SetItemsProcessed(items);
}

BENCHMARK(Doit);

BENCHMARK_MAIN();


