#include <Socket.h>
#include <Timer.h>
#include <inttypes.h>
#include <stdio.h>

#define MB1 (1024 * 1024)

int main(int argc, char *argv[]) {
  unsigned int rxb = 0;
  static int once = 1;

  Socket NMX(9000, 1500);
  Timer tm;

  for (;;) {
    rxb += NMX.Receive();
    if (once) {
      tm.Start();
      once = 0;
    }

    if (rxb > MB1 * 750) {
      tm.Stop();
      break;
    }
  }

  printf("Received %d bytes in %" PRIu64 " usecs\n", rxb, tm.ElapsedUS());
  printf("Rate: %.2f Mbps\n",
         rxb * 8.0 / (tm.ElapsedUS() / 1000000.0) / 1000000);
}
