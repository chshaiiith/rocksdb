#include <random>
#include <limits>
#include "random_helper.h"

std::mt19937_64 globalGenerator;

void seedGenerator(unsigned int seed)
{
    globalGenerator.seed(seed);
}

void seedGeneratorRand()
{
    std::random_device rd;
    globalGenerator.seed(rd());
}

double uniform01()
{
    return std::generate_canonical<double, std::numeric_limits<double>::digits>(globalGenerator);
}

