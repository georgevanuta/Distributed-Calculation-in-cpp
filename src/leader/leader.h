#ifndef LEADER_H
#define LEADER_H

#include "../topology/topology.h"

#include <vector>


/*
            Contains functions used in the second part of
        the assignment which are to only be used by the leader.
*/
namespace leader
{
    // Generates the vector.
    std::vector<int> generateVector(const size_t DIM);


    // Prints the result in the format given in the assignment .pdf.
    void printResult(const std::vector<int> vec);
}

#endif