#include <Thread.h>
#include <unistd.h>

void input_thread(void) {
  for (;;)
    ;
}

void processing_thread(void) {
  for (;;)
    ;
}
void output_thread(void) {
  for (;;)
    ;
}

/** Launch pipeline threads, then sleep forever */
int main(int argc, char *argv[]) {
  Thread t1(12, output_thread);
  Thread t2(13, processing_thread);
  Thread t3(14, input_thread);

  while (1) {
    sleep(2);
  }

  return 0;
}
