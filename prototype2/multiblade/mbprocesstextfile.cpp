#include <iostream>
#include <iomanip>
#include <fstream>
#include <tclap/CmdLine.h>
#include <climits>
#include <sys/stat.h>

#include "mbcommon/multiBladeEventBuilder.h"
#include "mbcommon/dumpEventBuilderInfo.h"
#include "mbcommon/TextFile.h"

int main(int argc, const char** argv) {

    std::string ifile = "";
    std::string ofile = "";
    std::string opath = "";
    uint nevents = UINT_MAX;

    try {
        TCLAP::CmdLine cmd("Test program for the Multiblade event-builder.", ' ', "0.1");

        TCLAP::ValueArg<std::string> ifileArg("f", "ifile", "File to analyze", true, "homer", "string");
        cmd.add(ifileArg);
        TCLAP::ValueArg<std::string> ofileArg("o", "ofile", "Output file", false, "", "string");
        cmd.add(ofileArg);
        TCLAP::ValueArg<std::string> opathArg("p", "opath", "Output path", false, "", "string");
        cmd.add(opathArg);
        TCLAP::ValueArg<uint> nArg("n", "nevents", "Number of events to analyze", false, UINT_MAX, "integer");
        cmd.add(nArg);

        cmd.parse(argc, argv);

        // Get the input file name
        ifile = ifileArg.getValue();
        // Get the output file name
        ofile = ofileArg.getValue();
        // Get the path for the output file
        opath = opathArg.getValue();
        // Get number of events to process
        nevents = nArg.getValue();

    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }

    if (opath.size() == 0) {
        std::size_t path_pos = ifile.find_last_of("/");
        opath = ifile.substr(0, path_pos);
    }
    if (ofile.size() == 0) {
        std::size_t path_pos = ifile.find_last_of("/");
        ofile = ifile.substr(path_pos + 1);
        ofile.erase(ofile.find("."), ofile.npos);
        ofile.append("_processed.txt");
    }
    struct stat statbuf;
    if (stat(opath.data(), &statbuf) == -1) {
        std::cout << "Path '" << opath << "' does not exist - exiting\n";
        return -1;
    }

    std::cout << "\nMultiblade event builder test program.\n\n";
    std::cout << "File to be read : " << ifile << std::endl;
    std::cout << "Output file : " << opath+"/"+ofile << std::endl;
    std::cout << "Number of data-points to be processed : " << (nevents == UINT_MAX ? "All" : std::to_string(nevents))
              << std::endl;
    std::cout << "\n";

    multiBladeEventBuilder p;
    p.setVerbose(false);
    p.setUseWeightedAverage(false);

    TextFile data(ifile);

    TextFile::Entry entry = {0,0,0,0};

    uint processed = 0;

    std::ofstream output;
    output.open(opath+"/"+ofile);

    while (true)
    {
        try {
            entry = data.nextEntry();
        } catch (TextFile::eof e) {
            std::cout << "End of file reached." << std::endl;
            break;
        }

        processed++;

        if (p.addDataPoint(entry.chan, entry.adc, entry.time))
        {
            output << std::fixed << std::setprecision(8) << std::setw(11)
                   << p.getWirePosition() << " "
                   << p.getStripPosition() << " "
                   << p.getTimeStamp() << "\n";
        }

        if ((processed < nevents) && (nevents != UINT_MAX))
            break;
    }

    p.lastPoint();
    // Repeated code ! Yes I know its bad, but this is just a test program for multiBladeEventBuilder.
    output << std::fixed << std::setprecision(8) << std::setw(11)
           << p.getWirePosition() << " "
           << p.getStripPosition() << " "
           << p.getTimeStamp() << "\n";

    output.close();

    dumpEventBuilderInfo info;
    info.print(p);

    return 0;
}