/** Copyright (C) 2016 European Spallation Source ERIC */

#include <algorithm>
#include <cassert>
#include <cstring>
#include <common/Trace.h>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

static int load_file(std::string file, char * buffer) {
  struct stat buf;
  const int cal_size = 16384;

  std::fill_n((char *)&buf, sizeof(struct stat), 0);

  int fd = open(file.c_str(), O_RDONLY);
  if (fd < 0) {
    XTRACE(CMD, WAR, "file open() failed for %s\n", file.c_str());
    return -10;
  }

  if (fstat(fd, &buf) != 0) {
    XTRACE(CMD, ERR, "fstat() failed for %s\n", file.c_str());
    close(fd);
    return -11;
  }

  if (buf.st_size != 16384*2) {
    XTRACE(CMD, WAR, "file %s has wrong length: %d (should be %d)\n", file.c_str(), buf.st_size, 16384*2);
    close(fd);
    return -12;
  }

  if (read(fd, buffer, cal_size*2) != cal_size*2) {
    XTRACE(CMD, ERR, "read() from %s incomplete\n", file.c_str());
    close(fd);
    return -13;
  }
  XTRACE(CMD, INF, "Calibration file %s sucessfully read\n", file.c_str());
  close(fd);
  return 0;
}

static int load_calib(std::string calibration) {
  const int cal_size = 16384;
  char * wcal[cal_size * 2];
  char * gcal[cal_size * 2];

  XTRACE(CMD, ALW, "Attempt to load calibration %s\n", calibration.c_str());

  auto file = calibration + std::string(".wcal");
  if (load_file(file, (char*)&wcal) < 0) {
    return -1;
  }
  file = calibration + std::string(".gcal");
  if (load_file(file, (char*)&gcal) < 0) {
    return -1;
  }
  return 0;
}

int Parser::parse(char * input, unsigned int ibytes, char * output, unsigned int * obytes) {
  XTRACE(CMD, DEB, "parse() received %u bytes\n", ibytes);
  *obytes = 0;
  if (ibytes == 0) {
    return -EUSIZE;
  }
  if (ibytes > SERVER_BUFFER_SIZE) {
    return -EOSIZE;
  }

  if (input[ibytes - 1] != '\0') {
    XTRACE(CMD, DEB, "adding null termination\n");
    auto end = std::min(ibytes, SERVER_BUFFER_SIZE - 1);
    input[end] = '\0';
  }

  std::vector<std::string> tokens;
  char *chars_array = strtok((char *)input, "\n ");
  while (chars_array) {
    std::string token(chars_array);
    tokens.push_back(token);
    chars_array = strtok(NULL, "\n ");
  }

  if ((int)tokens.size() < 1) {
    return -ENOTOKENS;
  }

  XTRACE(CMD, DEB, "Tokens in command: %d\n", (int)tokens.size());
  for (auto token : tokens) {
    XTRACE(CMD, INF, "Token: %s\n", token.c_str());
  }


  /** @todo This is really ugly, consider using another approach later */
  if (tokens.at(0).compare(std::string("STAT_INPUT")) == 0) {
    XTRACE(CMD, INF, "STAT_INPUT\n");
    *obytes = snprintf(output, SERVER_BUFFER_SIZE,
        "STAT_INPUT %" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64,
        opts.stat.i.rx_packets, opts.stat.i.rx_bytes,
        opts.stat.i.fifo_push_errors, opts.stat.i.fifo_free);

  } else if (tokens.at(0).compare(std::string("STAT_PROCESSING")) == 0) {
    XTRACE(CMD, INF, "STAT_PROCESSING\n");
    *obytes = snprintf(output, SERVER_BUFFER_SIZE,
                 "STAT_PROCESSING %" PRIu64 ", %" PRIu64 ", %" PRIu64
                 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64,
                 opts.stat.p.rx_events, opts.stat.p.rx_error_bytes,
                 opts.stat.p.rx_discards, opts.stat.p.rx_idle,
                 opts.stat.p.fifo_push_errors, opts.stat.p.fifo_free);

  } else if (tokens.at(0).compare(std::string("STAT_OUTPUT")) == 0) {
    XTRACE(CMD, INF, "STAT_OUTPUT\n");
    *obytes = snprintf(output, SERVER_BUFFER_SIZE,
        "STAT_OUTPUT %" PRIu64 ", %" PRIu64 ", %" PRIu64,
        opts.stat.o.rx_events, opts.stat.o.rx_idle, opts.stat.o.tx_bytes);

  } else if (tokens.at(0).compare(std::string("STAT_RESET")) == 0) {
    XTRACE(CMD, INF, "STAT_RESET\n");
    opts.stat.clear();
    *obytes = snprintf(output, SERVER_BUFFER_SIZE, "<OK>");

  } else if (tokens.at(0).compare(std::string("STAT_MASK")) == 0) {
    if ((int)tokens.size() != 2) {
      XTRACE(CMD, INF, "STAT_MASK wrong number of arguments\n");
      return -EBADARGS;
    }
    unsigned int mask = (unsigned int)std::stoul(tokens.at(1), nullptr, 0);
    XTRACE(CMD, WAR, "STAT_MASK 0x%08x\n", mask);
    opts.stat.set_mask(mask);
    *obytes = snprintf(output, SERVER_BUFFER_SIZE, "<OK>");


  } else if (tokens.at(0).compare(std::string("CSPEC_LOAD_CALIB")) == 0) {
    if ((int)tokens.size() != 2) {
      XTRACE(CMD, INF, "CSPEC_LOAD_CALIB wrong number of arguments\n");
      return -EBADARGS;
    }
    XTRACE(CMD, INF, "CSPEC_LOAD_CALIB\n");
    auto ret = load_calib(tokens.at(1));
    if (ret < 0) {
      *obytes = snprintf(output, SERVER_BUFFER_SIZE, "<ERROR>");
      return -EBADARGS;
    }
    *obytes = snprintf(output, SERVER_BUFFER_SIZE, "<OK>");

  } else {
    return -EBADCMD;
  }
  return 0;
}
