/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <benchmark/benchmark.h>
#include <gdgem/nmx/Hists.h> // @fixme
#include <multigrid/mgmesytec/Data.h>
#include <multigrid/mgmesytec/TestData.h>

static void Doit(benchmark::State &state)
{
  NMXHists hists;
  Producer producer {"noserver", "nostream"};
  MesytecData mesytec{0, "nofile", 1}; // Dont dumptofile select module with 20 depth in z
  ReadoutSerializer * serializer;
  FBSerializer * fbserializer;
  serializer = new ReadoutSerializer(10000, producer);
  fbserializer = new FBSerializer(1000000, producer);
  int SumReadouts = 0;
  for (auto _ : state) {
	  mesytec.parse((char *)&ws4[0], ws4.size(), hists, *fbserializer, *serializer);
    SumReadouts += mesytec.readouts;
  }
  state.SetItemsProcessed(SumReadouts);
  delete serializer;
  delete fbserializer;
}

BENCHMARK(Doit);

BENCHMARK_MAIN();
