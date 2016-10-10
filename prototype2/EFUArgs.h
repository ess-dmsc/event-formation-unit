/** Copyright (C) 2016 European Spallation Source */

#include <string>

class EFUArgs {
public:
  EFUArgs(int argc, char *argv[]);

  std::string dest_ip{"127.0.0.1"}; /**< used for data generators */
  int port{9000};                   /**< udp receive port */
  int buflen{9000};                 /**< rx buffer length (B) */
  int rcvbuf{1000000};              /**< socket rx buffer size (rmem_max) */
  int sndbuf{1000000};              /**< soxket tx buffer size (wmem_max) */

  unsigned int updint{1};             /**< update interval (s) */
  unsigned int stopafter{0xffffffff}; /**< 'never' stop */

  std::string det{"nmx"}; /**< detector name */

  // NMX OPTIONS
  int reduction{80}; /**< data tuples in a cluster */

  std::string broker{"localhost:9092"}; /**< Kafka broker */
  bool kafka{true};                     /**< whether to use Kafka or not */
};
