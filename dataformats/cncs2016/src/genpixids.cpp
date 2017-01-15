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

  printf("reading event data from file %s ...\n", eventfile.c_str());

  FILE * ff = fopen(eventfile.c_str(), "r");
  if (ff == NULL) {
    printf("  reading failed, check if file %s exist and has read permissions\n",
           eventfile.c_str());
    exit(1);
  }

  assert(CNCS.getmaxpixelid() == 6144);
  int values[6144]; /** size is specific to CNCS object above */
  int xyproj[8][48];
  int zyproj[48][16];
  int xzproj[8][16];
  memset(values, 0, sizeof(values));
  memset(xyproj, 0, sizeof(xyproj));
  memset(zyproj, 0, sizeof(zyproj));
  memset(xzproj, 0, sizeof(xzproj));

  int events = 0;
  int badpixels = 0;
  int goodpixels = 0;

  DataSave coords(eventfile + ".coord");
  DataSave xydata(eventfile + ".xyproj");
  DataSave zydata(eventfile + ".zyproj");
  DataSave xzdata(eventfile + ".xzproj");
  coords.tofile(std::string("# time,    wire, grid, pixl,   x,   y,  z\n"));

  int data[10];     /** holds channel data + time */
  while (1) {
    ret = fscanf(ff, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", &data[0],
                 &data[1], &data[2], &data[3], &data[4], &data[5], &data[6],
                 &data[7], &data[8], &data[9]);

    if (ret == EOF) {
      break;
    }

    events++;

    int w0pos = data[4]; /** fifth field of the .events file */
    int wireid = wcal[w0pos];
    if (wireid & 1) {
      wireid++;
    } else {
      wireid--;
    }

    int g0pos = data[8]; /** ninth field of the .events file */
    int gridid = gcal[g0pos];
    if (gridid <= 48) { /** swap modules @todo verify */
      gridid += 48;
    } else {
      gridid -= 48;
    }

    auto time = data[1];

    int pixid = CNCS.getdetectorpixelid(1, gridid, wireid);
    if (pixid != -1) { /**< create intensity volume image */
      auto x = CNCS.getxcoord(pixid);
      auto y = CNCS.getycoord(pixid);
      auto z = CNCS.getzcoord(pixid);
      assert(x>=1 && x <= 8);
      assert(y>=1 && y <= 48);
      assert(z>=1 && z <= 16);

      goodpixels++;
      values[pixid - 1]++; // 3D volume
      xyproj[x-1][y-1]++;
      zyproj[z-1][y-1]++;
      xzproj[x-1][15 - (z-1)]++;

      char buf[1000]; /**< @todo should be done in tofile() method */
      auto len = snprintf(buf, 1000, "%8d, %5d, %4d, %4d, %3d, %3d, %2d\n", time, wireid, gridid, pixid, x, y, z);
      coords.tofile(buf, len);
    } else {
      badpixels++;
    }
  }

  //for (int i = 0; i < 6144; i++) {
  //  printf("voxel: %4d, intensity %6d\n", i + 1, values[i]);
  //}

  for (int y = 0; y < 48; y++) {
    for (int x = 0; x < 8; x++) {
      xydata.tofile(std::to_string(xyproj[x][y]) + " ");
    }
    for (int z = 0; z < 16; z++) {
      zydata.tofile(std::to_string(zyproj[z][y]) + " ");
    }
    xydata.tofile("\n");
    zydata.tofile("\n");
  }

  for (int z = 0; z < 16; z++) {
    for (int x = 0; x < 8; x++) {
      xzdata.tofile(std::to_string(xzproj[x][z]) + " ");
    }
    xzdata.tofile("\n");
  }

  assert(events == goodpixels + badpixels);
  printf("Events:     %8d\nGoodpixels: %8d\nBadpixels:  %8d (%.2f%s)\n",
         events, goodpixels, badpixels, badpixels*100.0/events, "%");

  DataSave(outputfile, values, sizeof(values));

  return 0;
}
