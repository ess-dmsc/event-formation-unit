/** Copyright (C) 2018 European Spallation Source ERIC */

#include <benchmark/benchmark.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <gdgem/vmm2srs/SRSMappings.h>
#include <gdgem/vmm2srs/SRSTime.h>
#include <gdgem/dg_impl/NMXClusterer.h>
#include <gdgem/dg_impl/TestData.h>
#include <test/TestBase.h>

static void Doit(benchmark::State &state) {
	SRSMappings mapping;

	mapping.set_mapping(1, 0, 0, 0);
	mapping.set_mapping(1, 1, 0, 64);
	mapping.set_mapping(1, 6, 0, 128);
	mapping.set_mapping(1, 7, 0, 192);

	mapping.set_mapping(1, 10, 1, 0);
	mapping.set_mapping(1, 11, 1, 64);
	mapping.set_mapping(1, 14, 1, 128);
	mapping.set_mapping(1, 15, 1, 192);

	SRSTime srstime;
	srstime.set_bc_clock(20);
	srstime.set_tac_slope(60);
	srstime.set_trigger_resolution(3.125);

	int pAcqWin = 4000;
	int pADCThreshold = 0;
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


	NMXClusterer nmxdata(srstime, mapping, pAcqWin, pADCThreshold,
			pMinClusterSize, pDeltaTimeHits, pDeltaStripHits, pDeltaTimeSpan,
			pDeltaTimePlanes);
	for (auto _ : state) {
		for (auto hit : Run16_line_110168_110323) { // replace with UDP receive()
			int result = nmxdata.AnalyzeHits(hit.srs_timestamp,
					hit.framecounter, hit.fec, hit.chip_id, hit.channel,
					hit.bcid, hit.tdc, hit.adc, hit.overthreshold);

			if (result == -1) {
				printf("result == -1\n");
				break;
			}
		}
		//ASSERT_TRUE(nmxdata.getNumClustersX() < 3);
		//printf("getNumClustersX(): %d\n", nmxdata.getNumClustersX());	
		items += 156; // 156 hits in the Run16_line_110168_110323 dataset
	}
	// state.SetComplexityN(state.range(0));
	//state.SetBytesProcessed(state.iterations() * state.range(0));
	state.SetItemsProcessed(items);
}

BENCHMARK (Doit);

BENCHMARK_MAIN();
