#ifndef COORDINATORS_H
#define COORDINATORS_H

#include <set>
#include <string>
#include <map>
#include <vector>


// Contains functions used to deal with coordinators.
namespace crd 
{
    // Constant set containing the ranks of the coordinators.
    const std::set<int> COORDINATORS = { 0, 1, 2, 3 };

    // Constant rank which determines the leader of the coordinators.
    constexpr int LEADER = 0;


    // Constants to determine the kind of error in the topology.
    constexpr unsigned int ALL_FULL_CONNECTIVITY = 0;
    constexpr unsigned int ALL_PARTIAL_CONNECTIVITY = 1;
    constexpr unsigned int PARTITION_CONECTIVITY = 2;


    // Cluster type containing the coordinator and its members.
    typedef struct cluster_t
    {
        int coordinator;
        std::set<int> otherMembers;

        // construct by constructing each member
        cluster_t(int coord = -1, std::set<int> membs = {}) : 
            coordinator(coord), otherMembers(membs) {}

        // construct by serializing
        cluster_t(const int* clusterSerialized, const size_t count)
        {
            coordinator = clusterSerialized[0];
            otherMembers = {};
            for (size_t i = 1; i < count; i++)
            {
                otherMembers.insert(clusterSerialized[i]);
            }
        }

    } cluster_t;


    // Returns the last coordinator from the partition which contains the leader.
    const int getLastCoordinator(const unsigned int ERROR_TYPE);


    // Returns a mapping from coordinator to its coordinator neighbors.
    const std::map<int, std::vector<int>> getCoordNeighbors(const unsigned int ERROR_TYPE);


    // Checks if a process is a coordinator.
    const bool isCoordinator(const int rank);


    /*
            Opens an input file based on the process's rank and returns 
        a set containing its cluster.
    */    
    const cluster_t readCluster(const int rank);


    /*
            Used to announce the followers of a cluster who is the
        coordinator.
    */ 
    void announceFollowers(const cluster_t cluster);
}


#endif