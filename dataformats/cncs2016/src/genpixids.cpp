/** Copyright (C) 2016 European Spallation Source ERIC */

#include <DataSave.h>
#include <MapFile.h>
#include <common/MultiGridGeometry.h>
#include <cassert>
#include <cspec/CSPECChanConv.h>
#include <cspec/CalibrationFile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

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

  //
  printf("reading event data...\n");

  FILE * ff = fopen(eventfile.c_str(), "r");
  assert(ff != NULL);

  int data[10];     /** holds channel data + time */
  int values[6144]; /** size is specific to CNCS object above */
  memset(values, 0, 6144);

  while (1) {
    ret = fscanf(ff, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
               &data[0], &data[1], &data[2], &data[3], &data[4],
               &data[5], &data[6], &data[7], &data[8], &data[9]);

    if (ret == EOF) {
      break;
    }
    //printf("string: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
    //      data[0], data[1], data[2], data[3], data[4],
    //      data[5], data[6], data[7], data[7], data[9]);

    int w0pos = data[4]; /** fifth field of the .events file */
    int g0pos = data[8]; /** ninth field of the .events file */
    int gridid = 97 - gcal[g0pos]; /** reverse grids @todo verify */
    int wireid = wcal[w0pos]; 

    int pixid = CNCS.getdetectorpixelid(0, gridid, wireid);
    if (pixid != -1) {
      values[pixid]++;
    }
    printf("event: %d, wirepos %d, wire: %d, gridpos %d, grid: %d, pixel: %d\n",
           data[0], w0pos, wireid, g0pos, gridid, pixid);
  }

  DataSave("voxel.bin", values, sizeof(values));

  return 0;
}
