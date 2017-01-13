/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <DataSave.h>
#include <MapFile.h>
#include <cassert>
#include <common/MultiGridGeometry.h>
#include <cspec/CSPECChanConv.h>
#include <cspec/CalibrationFile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

int main(int argc, char *argv[]) {

  std::string eventfile, calibfile, outputfile;

  if (argc == 2) {
    std::string basename(argv[1]);
    eventfile = basename + ".events";
    calibfile = basename;
    outputfile = eventfile + ".vox";
  } else if (argc == 4) {
    eventfile = argv[1];
    calibfile = argv[2];
    outputfile = argv[3];
  } else {
    printf("Generate pixel ids from an event file and a calibration file.\n");
    printf("usage:\n");
    printf("  genpixids basename\n");
    printf("   - use basename.events and basename.wcal/basename.gcal\n");
    printf("  genpixids eventfile calibfile voxfile\n");
    printf("   - use eventfile and calibfile.wcal/calibfile.gcal\n");
    exit(1);
  }

  CalibrationFile calibio;
  uint16_t wcal[CSPECChanConv::adcsize];
  uint16_t gcal[CSPECChanConv::adcsize];

  printf("Loading calibration data from file %s ...\n", calibfile.c_str());
  auto ret = calibio.load(calibfile, (char *)wcal, (char *)gcal);
  if (ret != 0) {
    printf("  loading failed, check if files %s.wcal and \n  %s.gcal exist and have correct sizes\n",
            calibfile.c_str(), calibfile.c_str());
    exit(1);
  }

  printf("Creating MultiGrid parser objects..\n");
  CSPECChanConv calibration;
  MultiGridGeometry CNCS(1, 2, 48, 4, 16);

  printf("Applying calibration data..\n");
  calibration.load_calibration(wcal, gcal);

  //
  printf("reading event data from file %s ...\n", eventfile.c_str());

  FILE * ff = fopen(eventfile.c_str(), "r");
  if (ff == NULL) {
    printf("  reading failed, check if file %s exist and has read permissions\n",
           eventfile.c_str());
    exit(1);
  }

  int data[10];     /** holds channel data + time */
  int values[6144]; /** size is specific to CNCS object above */
  memset(values, 0, 6144);

  int events = 0;
  int errids = 0;
  while (1) {
    ret = fscanf(ff, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", &data[0],
                 &data[1], &data[2], &data[3], &data[4], &data[5], &data[6],
                 &data[7], &data[8], &data[9]);

    if (ret == EOF) {
      break;
    }

    events++;
    int w0pos = data[4]; /** fifth field of the .events file */
    int g0pos = data[8]; /** ninth field of the .events file */
    int wireid = wcal[w0pos];
    int gridid = gcal[g0pos];

    if (wireid & 1) {
      wireid++;
    } else {
      wireid--;
    }

    if (gridid <= 48) /** reverse grids @todo verify */
      gridid += 48; // swap modules
    else
      gridid -= 48; // swap modules

    int pixid = CNCS.getdetectorpixelid(1, gridid, wireid);

    auto time = data[1];
    auto x = (pixid - 1) / (48 * 16) + 1;
    auto y = ((pixid - 1) / 16 ) % 48 + 1;
    auto z = (pixid - 1) % 16 + 1;

    if (pixid != -1) { /**< create intensity volume image */
      values[pixid - 1]++;
      printf("%8d, w %5d, g %4d, p: %4d, x %3d, y %3d, z %3d\n", time, wireid, gridid, pixid, x, y, z);
    } else {
      errids++;
    }
    //printf("event: %6d, wirepos %4d, wire: %3d, gridpos %4d, grid: %3d, pixel: %4d\n",
    //       data[0], w0pos, wireid, g0pos, gridid, pixid);
  }

  //for (int i = 0; i < 6144; i++) {
  //  printf("voxel: %4d, intensity %6d\n", i + 1, values[i]);
  //}

  printf("generated %d pixels, discarded %d.\n", events, errids);

  DataSave(outputfile, values, sizeof(values));

  return 0;
}
