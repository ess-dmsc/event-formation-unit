

class EFUArgs {
public:
  EFUArgs(int argc, char *argv[]);

  int port{9000};   /**< udp receive port */
  int buflen{9000}; /**< rx buffer length (B) */

  int updint{1}; /**< update interval (s) */
};
