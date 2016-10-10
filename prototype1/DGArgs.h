/** Copyright (C) 2016 European Spallation Source */

#include <string>

class DGArgs {
public:
  DGArgs(int argc, char *argv[]);

  unsigned int txGB{10}; /**< total transmit size (GB) */
  int vmmtuples{20};     /**< number of tuples per packet */

  std::string dest_ip{"127.0.0.1"}; /**< destination ip address */
  int port{9000};                   /**< destination udp port */
  int buflen{9000};                 /**< Tx buffer size */
  int sndbuf{1000000};              /**< kernel sndbuf size */

  unsigned int updint{1}; /**< update interval (seconds) */
};
