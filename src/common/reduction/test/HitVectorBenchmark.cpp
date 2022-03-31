#include <benchmark/benchmark.h>
#include <common/reduction/HitVector.h>


static void BM_pushback_hits(benchmark::State& state){
	HitVector hits;
  	for (auto _ : state){
    	Hit hit;
    	hit.coordinate = 50;
    	hit.weight = 50;
    	hit.time = 50;
    	hits.push_back(hit);
  	}
}

BENCHMARK(BM_pushback_hits);
