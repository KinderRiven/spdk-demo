/*
 * @Author: your name
 * @Date: 2021-04-29 18:59:13
 * @LastEditTime: 2021-04-29 18:59:47
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /spdk-demo/timer.h
 */
#ifndef UTIL_TIMER_H_
#define UTIL_TIMER_H_

#include <stdint.h>
#include <time.h>

// #define RDTSC_CLOCK
#define CPU_SPEED_MHZ (2600)

class Timer {
public:
    unsigned long long asm_rdtsc(void)
    {
        unsigned hi, lo;
        __asm__ __volatile__("rdtsc"
                             : "=a"(lo), "=d"(hi));
        return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
    }

    unsigned long long asm_rdtscp(void)
    {
        unsigned hi, lo;
        __asm__ __volatile__("rdtscp"
                             : "=a"(lo), "=d"(hi)::"rcx");
        return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
    }

    uint64_t cycles_to_ns(int cpu_speed_mhz, uint64_t cycles)
    {
        return (cycles * 1000 / cpu_speed_mhz);
    }

    uint64_t ns_to_cycles(int cpu_speed_mhz, uint64_t ns)
    {
        return (ns * cpu_speed_mhz / 1000);
    }

public:
    Timer(void)
        : elapsed { 0 }
    {
    }

    void Start(void)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
    }

    void Stop(void)
    {
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
    }

    size_t Get(void)
    {
        return elapsed;
    }

    double GetSeconds(void)
    {
        return elapsed / 1000000000.0;
    }

    size_t Now(void)
    {
        clock_gettime(CLOCK_MONOTONIC, &now);
        return now.tv_sec * 1000000000 + now.tv_nsec;
    }

    void Accumulate(void)
    {
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed += (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
    }

private:
    struct timespec start, end, now;
    size_t elapsed;
};

#endif // UTIL_TIMER_H_
