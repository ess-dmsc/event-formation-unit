//
// Created by soegaard on 8/25/17.
//

#pragma once

#include <vector>
#include <cstdint>

struct datapoint {
    uint8_t digi;
    uint8_t chan;
    uint16_t adc;
    uint32_t time;
};

class MBData {

public:

    MBData();

    unsigned int recieve(const char* /*void **/ buffer, unsigned int size);

    std::vector<datapoint> data;


private:

    //unsigned int datalength;

    //uint64_t digi_mask;
    //uint64_t chan_mask;
    //uint64_t ADC_mask;
    //uint64_t time_mask;


};
