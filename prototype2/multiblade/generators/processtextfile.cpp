/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <climits>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>
#include <tclap/CmdLine.h>

#include "generators/TextFile.h"
#include "common/EventBuilder.h"
// GCOVR_EXCL_START
int main(int argc, const char **argv) {

  std::string ifile;
  std::string ofile;
  std::string opath;
  uint nevents = UINT_MAX;
  bool weighted = false;

  try {
    TCLAP::CmdLine cmd("Test program for the Multiblade event-builder.", ' ',
                       "0.1");

    TCLAP::ValueArg<std::string> ifileArg("f", "ifile", "File to analyze", true,
                                          "data.txt", "string");
    cmd.add(ifileArg);
    TCLAP::ValueArg<std::string> ofileArg("o", "ofile", "Output file", false,
                                          "", "string");
    cmd.add(ofileArg);
    TCLAP::ValueArg<std::string> opathArg("p", "opath", "Output path", false,
                                          "", "string");
    cmd.add(opathArg);
    TCLAP::ValueArg<uint> nArg("n", "nevents", "Number of events to analyze",
                               false, UINT_MAX, "integer");
    cmd.add(nArg);
    TCLAP::ValueArg<bool> weightArg("w", "weighted",
                                    "Use weighted average to find position",
                                    false, false, "boolean");
    cmd.add(weightArg);

    cmd.parse(argc, argv);

    // Get the input file name
    ifile = ifileArg.getValue();
    // Get the output file name
    ofile = ofileArg.getValue();
    // Get the path for the output file
    opath = opathArg.getValue();
    // Get number of events to process
    nevents = nArg.getValue();
    // Use max ADC or weighted average
    weighted = weightArg.getValue();

  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId()
              << std::endl;
  }

  if (opath.empty()) {
    std::size_t path_pos = ifile.find_last_of('/');
    opath = ifile.substr(0, path_pos);
  }
  if (ofile.empty()) {
    std::size_t path_pos = ifile.find_last_of('/');
    ofile = ifile.substr(path_pos + 1);
    ofile.erase(ofile.find('.'), ofile.npos);
    ofile.append("_processed.txt");
  }
  struct stat statbuf;
  if (stat(opath.data(), &statbuf) == -1) {
    std::cout << "Path '" << opath << "' does not exist - exiting\n";
    return -1;
  }

  std::cout << "\nMultiblade event builder test program.\n\n";
  std::cout << "File to be read : " << ifile << std::endl;
  std::cout << "Output file : " << opath + "/" + ofile << std::endl;
  std::cout << "Number of events to be processed : "
            << (nevents == UINT_MAX ? "All" : std::to_string(nevents))
            << std::endl;
  std::cout << "\n";

  Multiblade::EventBuilder p;
  p.setUseWeightedAverage(weighted);

  Multiblade::TextFile data(ifile);

  Multiblade::TextFile::Entry entry = {0, 0, 0, 0};

  uint events = 0;

  std::ofstream output;
  output.open(opath + "/" + ofile);

  while (true) {
    try {
      entry = data.nextEntry();
    } catch (Multiblade::TextFile::eof &e) {
      std::cout << "End of file reached." << std::endl;
      break;
    }

    if (p.addDataPoint(entry.chan, entry.adc, entry.time)) {
      output << std::fixed << std::setprecision(8) << std::setw(11)
             << p.getWirePosition() << " " << p.getStripPosition() << " "
             << p.getTimeStamp() << "\n";

      events++;
    }

    if ((events + 1 >= nevents) && (nevents != UINT_MAX))
      break;
  }

  p.lastPoint();
  // Repeated code ! Yes I know its bad, but this is just a test program for
  // EventBuilder.
  output << std::fixed << std::setprecision(8) << std::setw(11)
         << p.getWirePosition() << " " << p.getStripPosition() << " "
         << p.getTimeStamp() << "\n";

  output.close();

  p.print();

  return 0;
}
// GCOVR_EXCL_STOP
