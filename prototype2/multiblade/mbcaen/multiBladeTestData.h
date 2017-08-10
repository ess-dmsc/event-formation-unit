//
// Test data to use with unit testing of multiBladeEventBuilder.
//
// Author: Carsten Søgaard, Niels Bohr Institute, University of Copenhagen
// e-mail: soegaard@nbi.dk

#ifndef MULTIBLADE_MULTIBLADETESTDATA_H_H
#define MULTIBLADE_MULTIBLADETESTDATA_H_H

#include <vector>

// Configuration values for testing the MultiBlade event builder.
// These values are directly linked to the test data. Change both accordingly.
// The values are : time-window [clock-cycles], # wire channels, # strip channels
std::vector<uint> config {

        185, 32, 32
};

// Test data for the MultiBlade event-builder.
// The data expects standard configuration of the event-builder (see config above).
// Each row consists of a data-point and the return value (true/false)
// Columns 1 & 2: Wire channel, strip channel
// Columns 3 & 4: ADC-value, clock-cycle number
// Column 4: Should 'addDataPoint' return true or false
//
// Do not change these values unless you know what you are doing. If changed, change also
// validation_weighted and validation_max below accordingly.
std::vector<uint> data {

        // Event # 1 - 1 data-point
        0,  32, 1300, 1000, 0,
        // Event # 2 - 2 data-points
        1,  33,  800, 2000, 1,
        2,  34, 1500, 2010, 0,
        // Event # 3 - 3 data-points
        3,  35,  800, 3000, 1,
        4,  36, 1700, 3010, 0,
        5,  37,  500, 3020, 0,
        // Event # 4 - 4 data-points
        6,  38,  400, 4000, 1,
        7,  39,  600, 4010, 0,
        8,  40,  750, 4020, 0,
        9,  41, 1800, 4030, 0,
        // Event # 5 - 5 data-points
        10, 42, 1800, 5000, 1,
        11, 43, 1000, 5010, 0,
        12, 44,  500, 5020, 0,
        13, 45,  600, 5030, 0,
        14, 46,  200, 5040, 0,
        // Final event - 1 data-point (After this event lastPoint() should be called)
        15, 47, 1800, 6000, 1
};

std::vector<double> validation_weighted {

        // This vector contains the expected values from the clustering process, when the location method
        // is set to weighted average (default).
        // The ordering must not be changed!
        // These values must only be changed when data (above) is changed - otherwise the test will fail!

        // Event # 1
        static_cast<double>(0),
        // Event # 2
        static_cast<double>(1*800+2*1500)/static_cast<double>(800+1500),
        // Event # 3
        static_cast<double>(3*800+4*1700+5*500)/ static_cast<double>(800+1700+500),
        // Event # 4
        static_cast<double>(6*400+7*600+8*750+9*1800)/ static_cast<double>(400+600+750+1800),
        // Event # 5
        static_cast<double>(10*1800+11*1000+12*500+13*600+14*200)/ static_cast<double>(1800+1000+500+600+200),
        // Final event
        static_cast<double>(15)
};

std::vector<double> validation_max {

        // This vector contains the expected values from the clustering process, when the location method
        // is set to max ADC ( call setUseWeightedAverage(false) ).
        // The ordering must not be changed!
        // These values must only be changed when data (above) is changed - otherwise the test will fail!

        // Event # 1
        static_cast<double>(0),
        // Event # 2
        static_cast<double>(2),
        // Event # 3
        static_cast<double>(4),
        // Event # 4
        static_cast<double>(9),
        // Event # 5
        static_cast<double>(10),
        // Final event
        static_cast<double>(15)
};

#endif //MULTIBLADE_MULTIBLADETESTDATA_H_H
