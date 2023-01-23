#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "../coordinators/coordinators.h"

#include <map>
#include <vector>


// Contains functions used to deal with the discovery of the topology.
namespace top
{   
    // Type alias for less typing.
    typedef std::map<int, std::vector<int>> topology_t;


    // In the case of a partition, this will be returned.
    constexpr int BELONGS_TO_PARTITION = -1;


    // The leader initialized the discovery.
    void discoverTopologyAsLeader(
        const int rank,
        int* clusterSerialized, 
        const size_t clusterSerializedSize, 
        const crd::cluster_t cluster,
        topology_t* topology);


    /*
            The last reachable coordinator sends the feedback wave.
        When the topology reaches it, it will be completed, so it can be sent
        back the way it came from.
    */
    void discoverTopologyAsLastCoordinator(
        const int rank,
        const int parent,
        std::vector<int> clustersSerializedUnited,
        topology_t* topology,
        const crd::cluster_t cluster);


    /*
            A normal coordinator (one which is neither the last or the leader)
        receives the current topology from its parent, adds its cluster and then
        forwards it to its child. After that it will wait for the full topology
        from its child and then send it to its parent.
    */
    void discoverTopologyAsNormalCoordinator(
        const int rank,
        int parent,
        int* child,
        topology_t* topology,
        const std::map<int, std::vector<int>> coordNeighs,
        const std::vector<int> clustersSerializedUnited,
        const crd::cluster_t cluster);


    /*
            Function which adds a cluster to the current topology.
    */
    void aggregateTopology(
        const int rank,
        int* parent,
        std::vector<int>* clustersSerializedUnited,
        const crd::cluster_t cluster);


    /*
            A follower will discover the topology from its cluster
        coordinator, only needing to receive once.
    */
    void discoverTopologyAsFollower(
        const int rank,
        int* coordinator, 
        topology_t* topology);


    /*
            Function which combines all of the previous functions.
    */
    const int discoverTopology(
        const int rank,
        const unsigned int ERROR_TYPE,
        topology_t* topology,
        int* parent,
        int* child);
}



#endif