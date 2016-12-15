#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class DetMultiGrid {
public:
  enum class hdr { DAT = 0x00, HDR, END = 0x03 };

  typedef struct {
    unsigned int n_words : 12;
    unsigned int adc_res : 3;
    unsigned int out_format : 1;
    unsigned int module_id : 8;
    unsigned int sub_header : 6;
    unsigned int header_sig : 2;
  } EventHeader;

  typedef struct {
    unsigned int adc_data : 14;
    unsigned int overflow : 1;
    unsigned int nop : 1;
    unsigned int channel : 5;
    unsigned int fix : 9;
    unsigned int data_sig : 2;
  } DataWord;

  typedef struct {
    unsigned int trigger : 30;
    unsigned int footer_sig : 2;
  } Footer;

  union {
    unsigned int value;
    EventHeader eh;
    DataWord dw;
    Footer ef;
  } data;

  unsigned int wthresh{230}; // Current values from Anton
  unsigned int gthresh{180}; //          -=-
  unsigned int zeropos{150}; // No specification

  const char *df[9] = {"w0 amp", "w1 amp", "w0 pos", "w1 pos", "g0 amp",
                       "g1 amp", "g0 pos", "g1 pos", "time  "};

  unsigned int readsz{sizeof data};
};

int main(int argc, char *argv[]) {
  DetMultiGrid det;
  char *filename;

  struct stat_t {
    int rx;     // file stats - Rx bytes
    int errors; // event stat - header errors
    int events; // event stat - number of events
    int zeropos;
    int noise;  //
    int multi;  // event stat - number of multi events
  } stat;

  unsigned int rxdata[9], maxdata[9], mindata[9]; // readout data

  if (argc == 2) { // Filename only
    filename = argv[1];
  } else if (argc == 4) {        // filename wthresh gthresh
    det.wthresh = atoi(argv[2]); // signed int to unsigned int
    det.gthresh = atoi(argv[3]);
    assert(det.wthresh < 16384); // check for negative numbers
    assert(det.gthresh < 16384);
    filename = argv[1];
  } else {
    printf("usage: %s filename [wire_thresh grid_thresh]\n", argv[0]);
    return -1;
  }

  bzero(&stat, sizeof(stat));
  memset(maxdata, 0x00, sizeof(maxdata));
  memset(mindata, 0xff, sizeof(mindata));

  printf("file: %s\n", filename);
  printf("wire thresh: %u\n", det.wthresh);
  printf("grid thresh: %u\n", det.gthresh);

  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    printf("error: cannot open file \'%s\'\n", filename);
    return -1;
  }

  assert(det.readsz == 4);
  while (fread(&det.data, det.readsz, 1, f) > 0) {
    stat.rx += det.readsz;

    if (det.data.eh.header_sig == (int)DetMultiGrid::hdr::HDR) { // Read Header
      //printf("0x%08x\n", det.data.value);
      int nread = det.data.eh.n_words;
      assert(nread == 9);

      bzero(rxdata, sizeof(rxdata));
      for (int j = 0; j < nread - 1; j++) {
        if (fread(&det.data, det.readsz, 1, f) > 0) { // Read Data
          //printf("0x%08x\n", det.data.value);
          stat.rx += det.readsz;

          if (det.data.dw.data_sig != (int)DetMultiGrid::hdr::DAT) {
            stat.errors+= j + 2; // discard data and header read so far
            continue;
          }
          auto ch = det.data.dw.channel;
          auto dat = det.data.dw.adc_data;
          rxdata[ch] = dat;
          maxdata[ch] = std::max(maxdata[ch], dat);
          mindata[ch] = std::min(mindata[ch], dat);
        }
      }

      // Read Footer
      if (fread(&det.data, det.readsz, 1, f) > 0) {
        //printf("0x%08xs\n", det.data.value);
        stat.rx += det.readsz;

        if (det.data.ef.footer_sig != (int)DetMultiGrid::hdr::END) {
          stat.errors+= 10; // discard last 10 reads
          continue;
        }
        maxdata[8] = std::max(maxdata[8], det.data.ef.trigger);
        mindata[8] = std::min(mindata[8], det.data.ef.trigger);

        // We have data - Process event
        // Discard noisy data
        if ((rxdata[0] <= det.wthresh) || (rxdata[4] <= det.gthresh)) {
          stat.noise++;
          continue;
        }

        if ((rxdata[2] == 0) || (rxdata[6] == 0)) {
          stat.zeropos++;
          continue;
        }

        // Detect and discard double neutron event
        if ((rxdata[1] >= det.wthresh) ) {
          stat.multi++;
          continue;
        }
        // Real event
        stat.events++;
      }
    } else {
      stat.errors++; // due to wrong header
    }
  }

  assert((stat.events + stat.zeropos + stat.multi + stat.noise)*40 == stat.rx - stat.errors*4);

  printf("=======================\nStats\n");
  printf("Bytes read:    %d\n", stat.rx);
  printf("Bytes error:   %d\n", stat.errors*4);
  printf("Total samples: %d\n", stat.multi + stat.noise + stat.zeropos + stat.events);
  printf("  events:      %d\n", stat.events);
  printf("Discarded: %d\n", stat.zeropos + stat.noise + stat.multi);
  printf("  zeropos:     %d\n", stat.zeropos);
  printf("  noise:       %d\n", stat.noise);
  printf("  double:      %d\n", stat.multi);
  printf("-----------------------\n");
  printf("Name       Min     Max\n");
  for (int j = 0; j < 9; j++) {
    printf("%s  %5u  %8u\n", det.df[j], mindata[j], maxdata[j]);
  }
  printf("-----------------------\n");
  return 0;
}
