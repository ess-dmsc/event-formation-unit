#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>

#define SHM_KEY  105205672
#define US_ONE_SEC 1000000
#define MAX_CPU 6
#define NSAMPLES 10
#define OFFSET 64
#define SHM_SIZE MAX_CPU * OFFSET

uint64_t rdtscp(int *chip, int *core)
{
  uint32_t a, d, c;
  __asm__ volatile("rdtscp" : "=a"(a), "=d"(d), "=c"(c));
  *chip = (c & 0xFFF000) >> 12;
  *core =  c & 0xFFF;
  return ((uint64_t)a) | (((uint64_t)d) << 32);
}

uint64_t setup_shared_memory()
{
  uint64_t shmaddr;
  int shmid;
  key_t shmkey = SHM_KEY;
  if  ((shmid = shmget(shmkey, SHM_SIZE, IPC_CREAT | 0666)) == -1) {
    printf("Error creating shared memory segment\n");
    exit(0);
  }

  if ((shmaddr = (uint64_t)shmat(shmid, 0, 0)) == 0) {
    printf("Invalid memory ptr\n");
    exit(0);
  }
  return shmaddr;
}


void writer(uint64_t addr)
{
  int sock0, core0, sock, core;
  unsigned long tsc = rdtscp(&sock0, &core0);
  printf("timer: %" PRIu64 ", (core %d, socket %d)\n", tsc, core0, sock0);
  while (1) {
    tsc = rdtscp(&sock, &core);

    if (core != core0 || sock != sock0) {
      printf("moving between cores not supported\n");
      exit(1);
    }

    volatile uint64_t * addrp = (uint64_t *)(addr + OFFSET * core0);
    *addrp = tsc;
  }
}


void reader(uint64_t addr, int index1, int index2) {
  uint64_t tsc[MAX_CPU][NSAMPLES];
  int64_t tdiff[MAX_CPU];

  printf("Sampling tsc counters\n");
  for (int n = 0; n < NSAMPLES; n++) {
    for (int i = 0; i < MAX_CPU; i++) {
      tsc[i][n] = *(uint64_t *)(addr + i * OFFSET);
    }
    usleep(US_ONE_SEC);
  }

  printf("Calculating averages\n");
  for (int i = 0; i < MAX_CPU; i++) {
    int64_t diff = 0;
    for (int n = 0; n < NSAMPLES; n++) {
      diff += tsc[i][n] -  tsc[0][n];
    }
    tdiff[i] = diff/NSAMPLES;
    printf("avg tsc diff. 0 - %d: %" PRIi64 "\n", i, tdiff[i]);
  }

  printf("Measuring time\n");
  printf("    dt0    dt1    dt2   dt3   dt4   dt5\n");
  while (1) {
    for (int i = 0; i < MAX_CPU; i++) {
      tsc[i][0] = *(uint64_t *)(addr + i * OFFSET);
    }
    for (int i = 0; i < MAX_CPU; i++) {
      printf("%6" PRIi64 " ", tsc[i][0] - tdiff[i] - tsc[0][0]);
    }
    printf("\n");
    usleep(US_ONE_SEC);
  }
}


int main(int argc, char *argv[]) {
  int cpu1, cpu2;

  if (argc != 1 && argc != 3) {
    printf("usage: ./tsctest cpu1 cpu2       # start reader\n");
    printf("       taskset -c cpu1 ./a.out   # start writer\n");
    exit(0);
  }

  uint64_t addr = setup_shared_memory();

  if (argc == 3) {
    int cpu1 = atoi(argv[1]);
    int cpu2 = atoi(argv[2]);
    printf("%d %d\n", cpu1, cpu2);
    reader(addr, cpu1, cpu2);
  } else if (argc == 1) {
    writer(addr);
  }
  return 0;
}
