#include <iostream>
#include <iomanip>
#include <fstream>
#include <tclap/CmdLine.h>
#include <climits>
#include <math.h>
#include <sys/stat.h>

#include <chrono>

#include "mbcommon/multiBladeEventBuilder.h"
#include "mbcommon/dumpEventBuilderInfo.h"
#include "mbcommon/TextFile.h"

#define UNUSED __attribute__((unused))

int main(int argc, const char** argv) {

    double_t UNUSED accwire = 0;
    double_t UNUSED accstrip = 0;
    double_t UNUSED acctime = 0;

    std::string ifile = "";
    uint npasses = 1;

    try {
        TCLAP::CmdLine cmd("Test program for the Multiblade event-builder.", ' ', "0.1");

        TCLAP::ValueArg<std::string> ifileArg("f", "ifile", "File to analyze", true, "homer", "string");
        cmd.add(ifileArg);
        TCLAP::ValueArg<uint> nArg("n", "npasses", "Number of data passes ", false, UINT_MAX, "integer");
        cmd.add(nArg);

        cmd.parse(argc, argv);

        // Get the input file name
        ifile = ifileArg.getValue();
        // Get number times to pass the data
        npasses = nArg.getValue();

    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }

    std::cout << "\nMultiblade event builder test program.\n\n";
    std::cout << "File to be read : " << ifile << std::endl;
    std::cout << "Number of times the data will be passed : " << npasses << std::endl;
    std::cout << "\n";

    multiBladeEventBuilder p;
    p.setUseWeightedAverage(false);

    typedef std::chrono::high_resolution_clock Clock;

    auto t0 = Clock::now();

    TextFile data(ifile);
    std::vector<TextFile::Entry> entries;
    try {
        entries = data.rest();
    } catch (TextFile::eof e) {
        std::cout << "End of file reached." << std::endl;
        return 0;
    }

    uint ipass = 0;

    uint64_t nevents = 0;

    auto t1 = Clock::now();
    while (ipass < npasses)
    {

        for (uint i = 0; i < entries.size(); i++) {
            bool cluster_complete = p.addDataPoint(entries[i].chan, entries[i].adc, entries[i].time);
            if (cluster_complete){
                accwire  += p.getWirePosition();
                accstrip += p.getStripPosition();
                acctime  += p.getTimeStamp();

                nevents++;
            }
        }
        p.lastPoint();

        ipass++;
    }
    auto t2 = Clock::now();

    auto loadtime = std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0);
    auto acc_time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
    auto ttime = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t0);

    dumpEventBuilderInfo info;
    info.print(p);

    long npoints = entries.size()*npasses;


    double_t avtime = static_cast<double>(acc_time.count())/static_cast<double_t >(npoints);
    double_t evtime = static_cast<double>(acc_time.count())/static_cast<double_t >(nevents);

    std::cout << "\nTime to load data [ns]           : " << loadtime.count() << std::endl;
    std::cout << "Average time per data-point [ns] : " << avtime << std::endl;
    std::cout << "Average time per event [ns]      : " << evtime << std::endl;
    std::cout << "Processing time [ns]             : " << acc_time.count() << std::endl;
    std::cout << "Total time [ns]                  : " << ttime.count() << std::endl;

    std::cout << "\nAccumulted wire  = " << std::setprecision(2) << accwire << std::endl;
    std::cout << "Accumulted strip = " << std::setprecision(2) << accstrip << std::endl;
    std::cout << "Accumulted time  = " << std::setprecision(2) << acctime << std::endl;

    return 0;
}
