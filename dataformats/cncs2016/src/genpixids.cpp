/** Copyright (C) 2016 European Spallation Source ERIC */

#include <MapFile.h>
#include <common/MultiGridGeometry.h>
#include <cspec/CSPECChanConv.h>
#include <cspec/CalibrationFile.h>
#include <stdlib.h>
#include <string>

//#define UNUSED __attribute__((unused))

int main(int argc, char *argv[]) {

  if (argc != 3) {
    printf("usage: genpixids eventfile calibfile\n");
    exit(1);
  }

  std::string eventfile(argv[1]);
  std::string calibfile(argv[2]);

  CalibrationFile lcf;
  uint16_t wcal[CSPECChanConv::adcsize];
  uint16_t gcal[CSPECChanConv::adcsize];

  printf("Loading calibration data..\n");
  auto ret = lcf.load(calibfile, (char *)wcal, (char *)gcal);
  if (ret != 0) {
    printf("  loading failed, check if file exists and have correct sizes\n");
    exit(1);
  }
  printf("Creating MultiGrid parser objects..\n");
  CSPECChanConv calibration;
  MultiGridGeometry CNCS(1, 2, 48, 4, 16, 1, 1);

  printf("Applying calibration data..\n");
  calibration.load_calibration(wcal, gcal);

  MapFile file(eventfile);

  return 0;
}
