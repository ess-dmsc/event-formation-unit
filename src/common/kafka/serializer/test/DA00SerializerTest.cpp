#include <common/kafka/serializer/DA00Serializer.h>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {

  std::vector<uint32_t> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  auto sender = da00_faltbuffers::Frame1DHistogramBuilder<uint32_t, double>(
      "some topic", 1'000'000 / 14, 71428, "intensity", "counts");
  sender.serialize(data);

  return 0;
}