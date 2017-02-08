/** Copyright (C) 2016, 2017 European Spallation Source ERIC */


#include <common/StatPublisher.h>


StatPublisher::StatPublisher(std::string ip, int port) {
  statdb = new TCPClient(ip.c_str(), port);
}

void StatPublisher::publish(EFUStats::stat_t * s) {
  char buffer[1000];
  int unixtime = (int)time(NULL);

  // INPUT
  int len = sprintf(buffer, "efu2.input.rx_bytes %" PRIu64 " %d\n", s->rx_bytes, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.input.rx_packets %" PRIu64 " %d\n", s->rx_packets, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.input.i2pfifo_dropped %" PRIu64 " %d\n", s->fifo1_push_errors, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.input.i2pfifo_free %" PRIu64 " %d\n", s->fifo1_free, unixtime);
  statdb->senddata(buffer, len);

  // PROCESSING
  len = sprintf(buffer, "efu2.processing.rx_readouts %" PRIu64 " %d\n", s->rx_readouts, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.processing.rx_error_bytes %" PRIu64 " %d\n", s->rx_error_bytes, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.processing.rx_discards %" PRIu64 " %d\n", s->rx_discards, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.processing.rx_idle %" PRIu64 " %d\n", s->rx_idle1, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.processing.geom_err %" PRIu64 " %d\n", s->geometry_errors, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.processing.p2ofifo_dropped %" PRIu64 " %d\n", s->fifo2_push_errors, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.processing.p2ofifo_free %" PRIu64 " %d\n", s->fifo2_free, unixtime);
  statdb->senddata(buffer, len);

  // OUTPUT
  len = sprintf(buffer, "efu2.output.rx_events %" PRIu64 " %d\n", s->rx_events, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.output.rx_idle %" PRIu64 " %d\n", s->rx_idle2, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.output.tx_bytes %" PRIu64 " %d\n", s->tx_bytes, unixtime);
  statdb->senddata(buffer, len);
}
