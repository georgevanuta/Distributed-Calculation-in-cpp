#ifndef CALCULATE_H
#define CALCULATE_H

#include "../topology/topology.h"

#include <vector>


namespace calc
{
    // The value which is used to multilply the generated vector.
    constexpr int MULT = 5;


    /*
            Called by coordinators. Given a big vector, a process will
        hold a portion of it directly proportional to its number of workers
        and forward the rest of it to its child coordinator.
    */
    std::vector<int> sliceAndShareVector(
        const int rank,
        const int child,
        const size_t accProcs,
        const top::topology_t topology,
        std::vector<int> vec);


    /*
            Receive the vector and the accProcs which holds how
        many workers worked on it until now.
    */
    std::vector<int> recvNextVector(
        const int rank,
        const int parent,
        size_t* accProcs);


    /*
            Function used by a coordinator in order to send the
        equal parts of the vector to its workers.
    */
    void shareVectorToFollowers(
        const int rank,
        const top::topology_t topology,
        std::vector<int> vec);


    /*
            Concatanates the current vector with the vector received.
    */
    std::vector<int> aggregateVectors(
        const int rank,
        const int child,
        std::vector<int> currentVec);


    /*
            Multiplies the given vector with <MULT>.
    */
    std::vector<int> transformVector(std::vector<int> vec);


    /*
            Send the transformed vector to the parent.
        Used by both followers and coordinators.
    */
    void shareTransformedVector(
        const int rank,
        const int parent,
        std::vector<int> vec);


    /*
            A follower will call this function when it needs
        to receive a slice of the vector.
    */
    std::vector<int> recvVecAsFollower(const int coord);


    /*
            A follower will call this function when it has
        finished transforming the vector and wants to send
        it to its coordinator.
    */
    void sendVecAsFollower(std::vector<int> vecTransformed, const int rank, const int coord);
    

    /*
            A coordinator will call this function in order
        to aggregate the transformed vectors of each of its
        workers into one big vector.
    */
    std::vector<int> aggregateFollowersVecs(const int rank, const top::topology_t topology);


    /*
                ---USED FOR DEBUGGING---
        Prints the rank and the workload of a process.
    */
    void printVec(const int rank, const std::vector<int> vec);
}

#endif