//
// Created by soegaard on 8/22/17.
//

#include <cinttypes>
#include <unistd.h>

#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/FBSerializer.h>
#include <common/NewStats.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>

#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>

#include <mbcommon/multiBladeEventBuilder.h>
#include <mbcaen/MBData.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

using namespace std;
using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "Multiblade detector with CAEN readout";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

class MBCAEN : public Detector {
public:
    MBCAEN(void *args);
    void input_thread();
    void processing_thread();

    int statsize();
    int64_t statvalue(size_t index);
    std::string &statname(size_t index);

    /** @todo figure out the right size  of the .._max_entries  */
    static const int eth_buffer_max_entries = 20000;
    static const int eth_buffer_size = 9000;
    static const int kafka_buffer_size = 124000; /**< events */

private:
    /** Shared between input_thread and processing_thread*/
    CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
    RingBuffer<eth_buffer_size> *eth_ringbuf;

    NewStats ns{"efu2.mbcaen."};

    struct {
        // Input Counters
        int64_t rx_packets;
        int64_t rx_bytes;
        int64_t fifo1_push_errors;
        int64_t pad[5];

        // Processing Counters
        int64_t rx_idle1;
        int64_t rx_readouts;
        int64_t tx_bytes;
        int64_t rx_events;
    } ALIGN(64) mystats;

    EFUArgs *opts;
};

MBCAEN::MBCAEN(void *args) {
    opts = (EFUArgs *)args;

    XTRACE(INIT, ALW, "Adding stats\n");
    // clang-format off
    ns.create("input.rx_packets",                &mystats.rx_packets);
    ns.create("input.rx_bytes",                  &mystats.rx_bytes);
    ns.create("input.fifo1_push_errors",         &mystats.fifo1_push_errors);
    ns.create("processing.rx_readouts",          &mystats.rx_readouts);
    ns.create("processing.rx_idle1",             &mystats.rx_idle1);
    ns.create("processing.tx_bytes",             &mystats.tx_bytes);
    ns.create("processing.rx_events",            &mystats.rx_events);
    // clang-format on

    XTRACE(INIT, ALW, "Creating %d Multiblade Rx ringbuffers of size %d\n",
           eth_buffer_max_entries, eth_buffer_size);
    eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries);
    assert(eth_ringbuf != 0);
}

int MBCAEN::statsize() { return ns.size(); }

int64_t MBCAEN::statvalue(size_t index) { return ns.value(index); }

std::string &MBCAEN::statname(size_t index) { return ns.name(index); }

void MBCAEN::input_thread() {
    /** Connection setup */
    Socket::Endpoint local(opts->ip_addr.c_str(), opts->port);
    UDPServer mbdata(local);
    mbdata.buflen(opts->buflen);
    mbdata.setbuffers(0, opts->rcvbuf);
    mbdata.printbuffers();
    mbdata.settimeout(0, 100000); // One tenth of a second

    int rdsize;
    Timer stop_timer;
    TSCTimer report_timer;
    for (;;) {
        unsigned int eth_index = eth_ringbuf->getindex();

        /** this is the processing step */
        if ((rdsize = mbdata.receive(eth_ringbuf->getdatabuffer(eth_index),
                                     eth_ringbuf->getmaxbufsize())) > 0) {

            XTRACE(PROCESS, DEB, "Received an udp packet\n");

            mystats.rx_packets++;
            mystats.rx_bytes += rdsize;
            eth_ringbuf->setdatalength(eth_index, rdsize);

            if (input2proc_fifo.push(eth_index) == false) {
                mystats.fifo1_push_errors++;
            } else {
                eth_ringbuf->nextbuffer();
            }
        }

        // Checking for exit
        if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {

            if (stop_timer.timeus() >= opts->stopafter * 1000000LU) {
                std::cout << "stopping input thread, timeus " << stop_timer.timeus()
                          << std::endl;
                return;
            }
            report_timer.now();
        }
    }
}

void MBCAEN::processing_thread() {

    //uint8_t ncassets = UINT8_MAX;
    uint8_t nwires   = 32;
    uint8_t nstrips  = 32;

    Producer eventprod(opts->broker, "MB_detector");
    FBSerializer flatbuffer(kafka_buffer_size, eventprod);

    multiBladeEventBuilder builder;
    builder.setNumberOfWireChannels(nwires);
    builder.setNumberOfStripChannels(nstrips);
    builder.setUseWeightedAverage(false);

    MBData mbdata;

    unsigned int data_index;
    Timer stopafter_clock;
    TSCTimer global_time, report_timer;
    while (1) {
        if ((input2proc_fifo.pop(data_index)) == false) {
            mystats.rx_idle1++;
            usleep(10);
        } else {
            auto dataptr = eth_ringbuf->getdatabuffer(data_index);
            auto datalen = eth_ringbuf->getdatalength(data_index);

            mbdata.recieve(dataptr, datalen);

            auto dat = mbdata.data;
            mystats.rx_readouts += dat.size();

            for (uint i = 0; i < dat.size(); i++) {

                auto dp = dat.at(i);

                if (dp.digi == UINT8_MAX && dp.chan == UINT8_MAX && dp.adc == UINT16_MAX && dp.time == UINT32_MAX)
                {
                    XTRACE(PROCESS, DEB, "Last point\n");
                    builder.lastPoint();
                    break;
                }

                if (builder.addDataPoint(dp.chan, dp.adc, dp.time)) {

                    // Calculate pixel ID:
                    // The pixel ID format will be ddwwss where
                    // dd = digitizer number
                    // ww = wire channel (1-32) [0 = no wire channel fired]
                    // ss = strip channel (1-32) [0 = no strip channel fired]
                    uint32_t pixel_id = 10000 * dp.digi;
                    pixel_id +=           100 * (builder.getWirePosition() + 1);
                    pixel_id +=             1 * (builder.getStripPosition() + 1);

                    XTRACE(PROCESS, DEB, "wire pos: %d, strip pos: %d, pixel_id: %d\n",
                             (int)(builder.getWirePosition()  + 1),
                             (int)(builder.getStripPosition() + 1),
                             pixel_id);

                    mystats.tx_bytes += flatbuffer.addevent(builder.getTimeStamp(), pixel_id);
                    mystats.rx_events++;
                }
            }
        }

        // Checking for exit
        if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {

            mystats.tx_bytes += flatbuffer.produce();

            if (stopafter_clock.timeus() >= opts->stopafter * 1000000LU) {
                std::cout << "stopping processing thread, timeus " << std::endl;
                return;
            }
            report_timer.now();
        }
    }
}

/** ----------------------------------------------------- */

class MBCAENFactory : DetectorFactory {
public:
    std::shared_ptr<Detector> create(void *args) {
        return std::shared_ptr<Detector>(new MBCAEN(args));
    }
};

MBCAENFactory Factory;
