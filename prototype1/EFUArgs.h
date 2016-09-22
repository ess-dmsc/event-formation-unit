#include <string>

class EFUArgs {
public:
  EFUArgs(int argc, char *argv[]);

  int reduction{80}; /**< data tuples in a cluster */

  int port{9000};                       /**< udp receive port */
  int buflen{9000};                     /**< rx buffer length (B) */
  std::string broker{"localhost:9092"}; /**< Kafka broker */
  bool kafka{true};                     /**< whether to use Kafka or not */

  unsigned int updint{1};             /**< update interval (s) */
  unsigned int stopafter{0xffffffff}; /**< 'never' stop */
};
