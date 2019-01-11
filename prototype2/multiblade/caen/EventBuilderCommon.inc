// THIS FUNCTION IS USED UNMODIFIED IN BOTH UNIT TEST
// AND BENCHMARK TEST - DO NOT MODIFY OR MOVE

// Create Hits for numberClusters clusters, for each cluster we
// generate hitsPerPlane hits in x and hitsPerPlane hits in y. All hits are separated
// in time and space: y-coords are offset from x-coords by 1
// just for the heck of it. Hits within a plane are separated by
// interCoordTimeGap and hits between planes are separated by
// interPlaneTimeGap. Finally clusters are separated by (timegap + 1)

void createHits(uint32_t numberClusters, uint8_t hitsPerPlane) {
  uint8_t plane_x{0};
  uint8_t plane_y{1};
  uint16_t hit_adc{4000};
  uint64_t interCoordTimeGap = 5;
  uint64_t interPlaneTimeGap = 70; // max for this test to pass

  uint64_t t = 0;
  for (uint32_t i = 0; i < numberClusters; i++) {
    uint16_t coordStart = i % 32;

    // x-plane Hits
    for (uint32_t j = 0; j < hitsPerPlane; j++) {
      builder.insert({t, uint16_t(coordStart + j) , hit_adc, plane_x});
      //printf("insert x-Hit @ t=%llu, (%u, %u)\n", t, uint16_t(coordStart + j), hit_adc);
         t += interCoordTimeGap;
    }
    t -= interCoordTimeGap; // no gap after the last in each plane

    t += interPlaneTimeGap;

    // y-plane Hits
    for (uint32_t j = 0; j < hitsPerPlane; j++) {
      //printf("insert y-Hit @ t=%llu, (%u, %u)\n", t, uint16_t(coordStart + 1 + j), hit_adc);
      builder.insert({t, uint16_t(coordStart + 1 + j), hit_adc, plane_y});
      t += interCoordTimeGap;
    }
    t -= interCoordTimeGap; // no gap after the last in each plane

    t+= timegap + 1;
  }
}
