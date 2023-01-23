#include "calculate.h"


#include "../processes/processes.h"

#include <mpi.h>
#include <algorithm>
#include <string>

std::vector<int> calc::sliceAndShareVector(
    const int rank,
    const int child,
    const size_t accProcs,
    const top::topology_t topology,
    std::vector<int> vec)
{
    size_t totalWorkers = 0;
    for (auto& el : topology) totalWorkers += el.second.size();

    // substract the workers from the parent process
    totalWorkers -= accProcs;

    size_t currentCoordWorkers = topology.at(rank).size();

    size_t currentCoordVecSize = vec.size() / totalWorkers * currentCoordWorkers;
    
    std::vector<int> currentCoordPart(vec.begin(), vec.begin() + currentCoordVecSize + 1);

    size_t actualAccProcs = accProcs + currentCoordWorkers;

    std::vector<int> nextCoordPart(vec.begin() + currentCoordVecSize + 1, vec.end());
    nextCoordPart.insert(nextCoordPart.begin(), actualAccProcs);

    MPI_Send(nextCoordPart.data(), nextCoordPart.size(), MPI_INT, child, 0, MPI_COMM_WORLD);
    proc::logMessage(rank, child);

    return currentCoordPart;
}


std::vector<int> calc::recvNextVector(
    const int rank,
    const int parent,
    size_t* accProcs)
{
    MPI_Status status;
    MPI_Probe(parent, 0, MPI_COMM_WORLD, &status);

    int sizeRecv;
    MPI_Get_count(&status, MPI_INT, &sizeRecv);

    int* vecRecv = (int*)malloc(sizeRecv * sizeof(int));
    MPI_Recv(vecRecv, sizeRecv, MPI_INT, parent, 0, MPI_COMM_WORLD, &status);

    *accProcs = vecRecv[0];

    std::vector<int> vec = {};

    for (int i = 1; i < sizeRecv; i++)
    {
        vec.push_back(vecRecv[i]);
    }

    return vec;
}


void calc::shareVectorToFollowers(
    const int rank,
    const top::topology_t topology,
    std::vector<int> vec)
{
    const std::vector<int> FOLLOWERS = topology.at(rank);
    const size_t NR_WORKERS = FOLLOWERS.size();
    const size_t SHARE_PER_WORKER = vec.size() / NR_WORKERS;
    
    std::vector<size_t> offsets = {};
    
    size_t acc = SHARE_PER_WORKER * NR_WORKERS;
    size_t i = 0;
    size_t start = 0;
    
    for (auto& follower : FOLLOWERS)
    {
        offsets.push_back(SHARE_PER_WORKER);
        if (acc < vec.size())
        {
            acc++;
            offsets[i]++;
        }

        std::vector<int> currentWorkerShare(vec.begin() + start, vec.begin() + start + offsets[i]);

        MPI_Send(currentWorkerShare.data(), currentWorkerShare.size(), MPI_INT, follower, 0, MPI_COMM_WORLD);
        proc::logMessage(rank, follower);
        
        start += offsets[i];
        i++;
    }
}


std::vector<int> calc::aggregateVectors(
    const int rank,
    const int child,
    std::vector<int> currentVec)
{
    MPI_Status status;
    MPI_Probe(child, 0, MPI_COMM_WORLD, &status);

    int sizeRecv;
    MPI_Get_count(&status, MPI_INT, &sizeRecv);
    
    int* vecRecv = (int*)malloc(sizeRecv * sizeof(int));
    MPI_Recv(vecRecv, sizeRecv, MPI_INT, child, 0, MPI_COMM_WORLD, &status);

    std::vector<int> childVec(vecRecv, vecRecv + sizeRecv);
    currentVec.insert(currentVec.end(), childVec.begin(), childVec.end());

    return currentVec;
}


std::vector<int> calc::transformVector(std::vector<int> vec)
{
    std::vector<int> vecTransformed = {};
    std::transform(vec.begin(), vec.end(), std::back_inserter(vecTransformed), [](int x) -> int {return x * calc::MULT;});

    return vecTransformed;
}


void calc::shareTransformedVector(
    const int rank,
    const int parent,
    std::vector<int> vec)
{
    MPI_Send(vec.data(), vec.size(), MPI_INT, parent, 0, MPI_COMM_WORLD);
    proc::logMessage(rank, parent);
}


std::vector<int> calc::recvVecAsFollower(const int coord)
{
    MPI_Status status;
    MPI_Probe(coord, 0, MPI_COMM_WORLD, &status);

    int sizeRecv;
    MPI_Get_count(&status, MPI_INT, &sizeRecv);

    int* vecRecv = (int*)malloc(sizeRecv * sizeof(int));
    MPI_Recv(vecRecv, sizeRecv, MPI_INT, coord, 0, MPI_COMM_WORLD, &status);

    std::vector<int> vec(vecRecv, vecRecv + sizeRecv);

    return vec;
}


void calc::sendVecAsFollower(std::vector<int> vecTransformed, const int rank, const int coord)
{
    MPI_Send(vecTransformed.data(), vecTransformed.size(), MPI_INT, coord, 0, MPI_COMM_WORLD);
    proc::logMessage(rank, coord);
}


std::vector<int> calc::aggregateFollowersVecs(const int rank, const top::topology_t topology)
{
    const std::vector<int> FOLLOWERS = topology.at(rank);
    std::vector<int> workersRes = {};

    for (auto& follower : FOLLOWERS)
    {
        workersRes = calc::aggregateVectors(rank, follower, workersRes);
    }

    return workersRes;
}


void calc::printVec(const int rank, const std::vector<int> vec)
{
    std::string vecString = std::to_string(rank) + " :::: " + std::to_string(vec.size()) + "\n";
    std::cout << vecString;
}