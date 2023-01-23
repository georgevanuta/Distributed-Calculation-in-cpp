#include "coordinators.h"

#include <fstream>
#include <mpi.h>

#include "../helpers/helpers.h"
#include "../processes/processes.h"
#include "../topology/topology.h"


const int crd::getLastCoordinator(const unsigned int ERROR_TYPE)
{
    return (ERROR_TYPE == crd::ALL_FULL_CONNECTIVITY || ERROR_TYPE == crd::ALL_PARTIAL_CONNECTIVITY) ? 1 : 2;
}


const std::map<int, std::vector<int>> crd::getCoordNeighbors(const unsigned int ERROR_TYPE)
{
    const unsigned int ALL_FULL_CONNECTIVITY = 0;
    const unsigned int ALL_PARTIAL_CONNECTIVITY = 1;
    const unsigned int PARTITION_CONECTIVITY = 2;

    std::map<int, std::vector<int>> coordNeighs;

    switch (ERROR_TYPE)
    {
        case ALL_FULL_CONNECTIVITY:
            coordNeighs =
            {
                { 0 , std::vector<int>( { 1 , 3 } ) },
                { 1 , std::vector<int>( { 0 , 2 } ) },
                { 2 , std::vector<int>( { 1 , 3 } ) },
                { 3 , std::vector<int>( { 0 , 2 } ) }
            };
            break;
        
        case ALL_PARTIAL_CONNECTIVITY:
            coordNeighs =
            {
                { 0 , std::vector<int>( { 3 } ) },
                { 1 , std::vector<int>( { 2 } ) },
                { 2 , std::vector<int>( { 1 , 3 } ) },
                { 3 , std::vector<int>( { 0 , 2 } ) }
            };
            break;

        case PARTITION_CONECTIVITY:
            coordNeighs =
            {
                { 0 , std::vector<int>( { 3 } ) },
                { 1 , std::vector<int>( { } ) },
                { 2 , std::vector<int>( { 3 } ) },
                { 3 , std::vector<int>( { 0 , 2 } ) }
            };
            break;

        default:
            helper::exitIf(true, "[ERROR]: <ERROR_TYPE> should have values between 0 and 2.\n");      
    }

    return coordNeighs;
}


const bool crd::isCoordinator(const int rank) 
{
    return crd::COORDINATORS.find(rank) != crd::COORDINATORS.end();
}


const crd::cluster_t crd::readCluster(const int rank) 
{
    std::set<int> clusterMembers = {};
    
    const std::string FILE_NAME = "cluster" + std::to_string(rank) + ".txt";
    std::ifstream inputFile(FILE_NAME);
    std::string lineStr;

    getline(inputFile, lineStr);
    const size_t NR_LINES = std::stoi(lineStr);

    for (size_t i = 0; i < NR_LINES; i++) {
        getline(inputFile, lineStr);
        const int CLUSTER_MEMBER = std::stoi(lineStr);
        clusterMembers.insert(CLUSTER_MEMBER);
    }

    crd::cluster_t cluster = cluster_t(rank, clusterMembers);

    return cluster;
}


void crd::announceFollowers(const crd::cluster_t cluster)
{   
    for (auto& member : cluster.otherMembers)
    {
        MPI_Send(&cluster.coordinator, 1, MPI_INT, member, 0, MPI_COMM_WORLD);
        proc::logMessage(cluster.coordinator, member);
    }
}
