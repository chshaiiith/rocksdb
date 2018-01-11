#ifndef RANDOM_HELPER_H
#define RANDOM_HELPER_H

#include <random>

extern std::mt19937_64 globalGenerator;

// Initialize random generator with seed
void seedGenerator(unsigned int seed);
// Initialize random generator with random seed
void seedGeneratorRand();

// Generate a uniform random number in the range [0, 1)
double uniform01();

#endif /* RANDOM_HELPER_H */

