#ifndef WORKLOAD_PROCESS_H
#define WORKLOAD_PROCESS_H

#include <cstdint>
#include "random_helper.h"

class Workload
{
public:
    // Deterministic TYPE = 1
    // Non-Deterministic TYPE = 2
    Workload(double rate, int type);
    double _rate;
    int _type;
    // Returns next interarrival arrival time (nanoseconds)
    double nextArrival();
};

#endif
