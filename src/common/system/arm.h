

static __inline__ unsigned long long rdtsc(void) {
    unsigned long long val;
    asm volatile("mrs %0, cntvct_el0" : "=r" (val));
    return val;
}