#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
//#include <StatCounter.h>

const char * df[] = {"w0 amp", "w1 amp", "w0 pos", "w1 pos", "g0 amp", "g1 amp", "g0 pos", "g1 pos", "time  "};

class DetMultiGrid {
public:
  enum class hdr { DAT = 0x00, HDR, END = 0x03  };

  typedef struct {
    unsigned int    n_words    : 12;
    unsigned int    adc_res    : 3;
    unsigned int    out_format : 1;
    unsigned int    module_id  : 8;
    unsigned int    sub_header : 6;
    unsigned int    header_sig : 2;
  } EventHeader;

typedef struct {
    unsigned int    adc_data   : 14;
    unsigned int    overflow   : 1;
    unsigned int    nop        : 1;
    unsigned int    channel    : 5;
    unsigned int    fix        : 9;
    unsigned int    data_sig   : 2;
  } DataWord;

  typedef struct {
    unsigned int    trigger    : 30;
    unsigned int    footer_sig : 2;
  } Footer;

  union {
    EventHeader eh;
    DataWord dw;
    Footer ef;
  } data;
};

int main(int argc, char * argv[]) {
  DetMultiGrid det;
  char * filename;
  unsigned int thresh = 150;

  struct stat_t {
    int rx;      // file stats - Rx bytes
    int errors;  // event stat - header errors
    int events;  // event stat - number of events
    int noise;   //
    int multi;   // event stat - number of multi events
  } stat;

  unsigned int rxdata[9], maxdata[9], mindata[9];  // readout data

  bzero(&stat, sizeof(stat));

  if (argc == 2) {  // Filename only
    filename = argv[1];
  } else if (argc == 3) {
    thresh = atoi(argv[1]);
    filename = argv[2];
  } else {
    printf("usage: %s [threshold] filename\n", argv[0]);
    return -1;
  }

  printf("file: %s\n", filename);
  printf("Threshold value: %d\n", thresh);


  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    printf("Cannot find file \'%s\'\n", argv[1]);
    return -1;
  }

  memset(maxdata, 0x00, sizeof(maxdata));
  memset(mindata, 0xff, sizeof(mindata));

  int readsz = sizeof det.data;
  while (fread(&det.data, readsz, 1, f) > 0){
    stat.rx += readsz;

    if (det.data.eh.header_sig == (int)DetMultiGrid::hdr::HDR) { // Read Header
      //printf("\n%7d Header 0x%02x, subhdr: %3d, module id: %3d, words: %3d\n",
      //        i, det.data.eh.header_sig, det.data.eh.sub_header, det.data.eh.module_id, det.data.eh.n_words);

      bzero(rxdata, sizeof(rxdata));
      int nread = det.data.eh.n_words;

      for (int j = 0; j < nread - 1; j++) {
        if (fread(&det.data, readsz, 1, f) > 0) {     // Read Data
          stat.rx += readsz;

          if (det.data.dw.data_sig != (int)DetMultiGrid::hdr::DAT) {
            printf("Data Error x%02x\n", det.data.dw.data_sig);
            stat.errors++;
            continue;
          }
          rxdata[det.data.dw.channel] = det.data.dw.adc_data;
          maxdata[det.data.dw.channel] = std::max(maxdata[det.data.dw.channel], det.data.dw.adc_data);
          mindata[det.data.dw.channel] = std::min(mindata[det.data.dw.channel], det.data.dw.adc_data);
          //printf("%7d Data:  %s : %5u\n", stat.events, df[det.data.dw.channel], det.data.dw.adc_data);
        }
      }

      // Read Footer
      if (fread(&det.data, readsz, 1, f) > 0) {
        stat.rx += readsz;

        if (det.data.ef.footer_sig != (int)DetMultiGrid::hdr::END) {
          printf("Footer Error: x%02x (bytes read: %d)\n", det.data.ef.footer_sig, stat.rx);
          stat.errors++;
          continue;
        }
        maxdata[8] = std::max(maxdata[8], det.data.ef.trigger);
        mindata[8] = std::min(mindata[8], det.data.ef.trigger);
        //printf("%7d Footer: timestamp %d\n", i, det.data.ef.trigger);


        // Process event
        if ((rxdata[0] < thresh) && (rxdata[4] < thresh)) {
          stat.noise++;
          continue;
        }

        stat.events++;

        if ((rxdata[1] > thresh) && (rxdata[5] > thresh)) {
          stat.multi++;
          printf("%7d: Double neutron Event (bytes read %d)\n", stat.events, stat.rx);
        }
      }
    }
  }
  printf("Bytes read:    %d\n", stat.rx);
  printf("Events:        %d\n", stat.events);
  printf("  double:      %d\n", stat.multi);
  printf("Non Events:    %d\n", stat.noise);
  printf("Errors:        %d\n", stat.errors);

  for (int j = 0; j < 9; j++) {
    printf("%s max: %5u, min: %5u\n", df[j], maxdata[j], mindata[j]);
  }
  return 0;
}
