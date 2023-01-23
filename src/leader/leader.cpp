#include "leader.h"

#include <algorithm>
#include <iostream>
#include <mpi.h>


std::vector<int> leader::generateVector(const size_t DIM)
{
    std::vector<int> vec(DIM);
    std::generate(vec.begin(), vec.end(), [n = DIM - 1] () mutable {return n--;});
    return vec;
}


void leader::printResult(const std::vector<int> vec)
{
    printf("Rezultat:");
    for (auto& el : vec)
    {
        printf(" %d", el);
    }
}