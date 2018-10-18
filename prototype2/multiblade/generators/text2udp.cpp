/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <iostream>
#include <sstream>

#include <cstring>
#include <iomanip>
#include <libs/include/Socket.h>
#include <generators/TextFile.h>
#include <generators/Args.h>

// GCOVR_EXCL_START

int main(int argc, char *argv[]) {
  Multiblade::Args opts(argc, argv);

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts.dest_ip.c_str(), opts.port);

  UDPTransmitter DataSource(local, remote);
  DataSource.setBufferSizes(opts.sndbuf, 0);
  DataSource.printBufferSizes();

  Multiblade::TextFile file(opts.filename);

  char buffer[9000];
  char *bufptr = buffer;

  uint64_t pkt = 0;
  uint64_t bytes = 0;

  uint linesize = sizeof(Multiblade::TextFile::Entry::digi) +
                  sizeof(Multiblade::TextFile::Entry::chan) + sizeof(Multiblade::TextFile::Entry::adc) +
                  sizeof(Multiblade::TextFile::Entry::time);

  // Multiblade::TextFile::Entry entry = {0, 0, 0, 0};
  Multiblade::TextFile::Entry end = {UINT8_MAX, UINT8_MAX, UINT16_MAX, UINT32_MAX};

  uint dppkg = 0;

  int pkgsize = 0;

  std::vector<Multiblade::TextFile::Entry> entries;
  try {
    entries = file.rest();
  } catch (Multiblade::TextFile::eof e) {
    std::cout << "End of file reached." << std::endl;
    return 0;
  }

  uint ievent = 0;

  do {
    std::memcpy(bufptr, &entries[ievent].digi, sizeof(Multiblade::TextFile::Entry::digi));
    bufptr += sizeof(Multiblade::TextFile::Entry::digi);

    std::memcpy(bufptr, &entries[ievent].chan, sizeof(Multiblade::TextFile::Entry::chan));
    bufptr += sizeof(Multiblade::TextFile::Entry::chan);

    std::memcpy(bufptr, &entries[ievent].adc, sizeof(Multiblade::TextFile::Entry::adc));
    bufptr += sizeof(Multiblade::TextFile::Entry::adc);

    std::memcpy(bufptr, &entries[ievent].time, sizeof(Multiblade::TextFile::Entry::time));
    bufptr += sizeof(Multiblade::TextFile::Entry::time);

    ievent++;
    if (ievent == entries.size())
      ievent = 0;

    pkgsize += linesize;

    dppkg++;

    if ((dppkg >= opts.dppkg) || (pkgsize >= opts.buflen)) {

      DataSource.send(buffer, pkgsize);

      bytes += pkgsize; // buflen;
      pkt++;

      bufptr = buffer;

      pkgsize = 0;
      dppkg = 0;
    }

  } while (pkt < opts.txPkt);

  std::memcpy(bufptr, &end.digi, sizeof(Multiblade::TextFile::Entry::digi));
  bufptr += sizeof(Multiblade::TextFile::Entry::digi);

  std::memcpy(bufptr, &end.chan, sizeof(Multiblade::TextFile::Entry::chan));
  bufptr += sizeof(Multiblade::TextFile::Entry::chan);

  std::memcpy(bufptr, &end.adc, sizeof(Multiblade::TextFile::Entry::adc));
  bufptr += sizeof(Multiblade::TextFile::Entry::adc);

  std::memcpy(bufptr, &end.time, sizeof(Multiblade::TextFile::Entry::time));
  bufptr += sizeof(Multiblade::TextFile::Entry::time);

  pkgsize += linesize;

  DataSource.send(buffer, pkgsize);

  bytes += pkgsize; // buflen;
  pkt++;

  printf("Sent: %" PRIu64 " packets\n", pkt);
  printf("Sent: %" PRIu64 " bytes\n", bytes);

  if (opts.outfile.empty() || opts.filename.empty())
    return 0;

  printf("Success creating\n");

  return 0;
}
// GCOVR_EXCL_STOP
