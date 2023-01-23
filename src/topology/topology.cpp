#include "topology.h"

#include "../processes/processes.h"
#include "../coordinators/coordinators.h"

#include <mpi.h>




void top::discoverTopologyAsLeader(
    const int rank,
    int* clusterSerialized, 
    const size_t clusterSerializedSize, 
    const crd::cluster_t cluster,
    topology_t* topology)
{
    const int CHILD = 3;

    MPI_Send(clusterSerialized, clusterSerializedSize, MPI_INT, CHILD, 0, MPI_COMM_WORLD);
    proc::logMessage(rank, CHILD);

    MPI_Status status;
    MPI_Probe(CHILD, 0, MPI_COMM_WORLD, &status);
    
    int sizeRecv;
    MPI_Get_count(&status, MPI_INT, &sizeRecv);

    int* topologySerialized = (int*)malloc(sizeRecv * sizeof(int));
    MPI_Recv(topologySerialized, sizeRecv, MPI_INT, CHILD, 0, MPI_COMM_WORLD, &status);

    *topology = proc::deserializeTopology(topologySerialized, sizeRecv);
    proc::logTopology(rank, *topology);
    for (auto& member : cluster.otherMembers)
    {
        MPI_Send(topologySerialized, sizeRecv, MPI_INT, member, 0, MPI_COMM_WORLD);
        proc::logMessage(rank, member);
    }
}


void top::discoverTopologyAsLastCoordinator(
    const int rank,
    const int parent,
    std::vector<int> clustersSerializedUnited,
    top::topology_t* topology,
    const crd::cluster_t cluster)
{
    MPI_Send(clustersSerializedUnited.data(), clustersSerializedUnited.size(), MPI_INT, parent, 0, MPI_COMM_WORLD);
    proc::logMessage(rank, parent);
    *topology = 
        proc::deserializeTopology(clustersSerializedUnited.data(), clustersSerializedUnited.size());

    proc::logTopology(rank, *topology);

    for (auto& member : cluster.otherMembers)
    {
        MPI_Send(clustersSerializedUnited.data(), clustersSerializedUnited.size(), MPI_INT, member, 0, MPI_COMM_WORLD);
        proc::logMessage(rank, member);
    }
}


void top::discoverTopologyAsNormalCoordinator(
        const int rank,
        const int parent,
        int* child,
        top::topology_t* topology,
        const std::map<int, std::vector<int>> coordNeighs,
        const std::vector<int> clustersSerializedUnited,
        const crd::cluster_t cluster)
{
    std::vector<int> neighbors = coordNeighs.at(rank);
    for (auto& neighbor : neighbors)
    {
        if (neighbor != parent)
        {
            MPI_Send(clustersSerializedUnited.data(), clustersSerializedUnited.size(), MPI_INT, neighbor, 0, MPI_COMM_WORLD);
            proc::logMessage(rank, neighbor);
            *child = neighbor;
            break;
        }
    }

    MPI_Status status;
    MPI_Probe(*child, 0, MPI_COMM_WORLD, &status);

    int sizeRecv;
    MPI_Get_count(&status, MPI_INT, &sizeRecv);

    int* topologySerialized = (int*)malloc(sizeRecv * sizeof(int));
    MPI_Recv(topologySerialized, sizeRecv, MPI_INT, *child, 0, MPI_COMM_WORLD, &status);           
    *topology = proc::deserializeTopology(topologySerialized, sizeRecv);
    proc::logTopology(rank, *topology);

    MPI_Send(topologySerialized, sizeRecv, MPI_INT, parent, 0, MPI_COMM_WORLD);
    proc::logMessage(rank, parent);

    for (auto& member : cluster.otherMembers)
    {
        MPI_Send(topologySerialized, sizeRecv, MPI_INT, member, 0, MPI_COMM_WORLD);
        proc::logMessage(rank, member);
    }
}



void top::discoverTopologyAsFollower(
    const int rank,
    int* coordinator, 
    top::topology_t* topology)
{
    MPI_Status status;
    MPI_Recv(coordinator, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

    int sizeRecv;
    MPI_Probe(*coordinator, 0, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &sizeRecv);

    int* fullTopology = (int*)malloc(sizeRecv * sizeof(int));
    MPI_Recv(fullTopology, sizeRecv, MPI_INT, *coordinator, 0, MPI_COMM_WORLD, &status);

    *topology = proc::deserializeTopology(fullTopology, sizeRecv);
    proc::logTopology(rank, *topology);
}


void top::aggregateTopology(
    const int rank,
    int* parent,
    std::vector<int>* clustersSerializedUnited,
    const crd::cluster_t cluster)
{
    MPI_Status status;
    MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    
    int sizeRecv;
    MPI_Get_count(&status, MPI_INT, &sizeRecv);

    int* clustersSerialized = (int*)malloc(sizeRecv * sizeof(int));
    MPI_Recv(clustersSerialized, sizeRecv, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    *parent = status.MPI_SOURCE;

    std::vector<int> clustersSerializedUnitedAux(clustersSerialized, clustersSerialized + sizeRecv);
    clustersSerializedUnitedAux.push_back(-1);
    clustersSerializedUnitedAux.push_back(cluster.coordinator);

    for (auto& member : cluster.otherMembers)
    {
        clustersSerializedUnitedAux.push_back(member);
    }

    *clustersSerializedUnited = clustersSerializedUnitedAux;
}


const int top::discoverTopology(
    const int rank,
    const unsigned int ERROR_TYPE,
    top::topology_t* topology,
    int* parent,
    int* child)
{
    if (crd::isCoordinator(rank))
    {
        *parent = -1;

        // Read your own cluster.
        const crd::cluster_t cluster = crd::readCluster(rank);

        // Serialize it.
        size_t clusterSerializedSize = 1 + cluster.otherMembers.size();
        int* clusterSerialized = (int*)malloc(clusterSerializedSize * sizeof(int));
        clusterSerialized[0] = rank;

        size_t i = 1;
        for (auto& member : cluster.otherMembers)
        {
            clusterSerialized[i] = member;
            i++;
        }

        // Announce your followers that you are the leader.
        crd::announceFollowers(cluster);


        // Handle partition.
        if (ERROR_TYPE == crd::PARTITION_CONECTIVITY && rank == 1)
        {
            *topology = proc::deserializeTopology(clusterSerialized, clusterSerializedSize);
            proc::logTopology(rank, *topology);
            for (auto& member : cluster.otherMembers)
            {
                MPI_Send(clusterSerialized, clusterSerializedSize, MPI_INT, member, 0, MPI_COMM_WORLD);
                proc::logMessage(rank, member);
            }
            MPI_Finalize();
            return top::BELONGS_TO_PARTITION;
        }

        const std::map<int, std::vector<int>> coordNeighs = 
            crd::getCoordNeighbors(ERROR_TYPE);


        if (rank == crd::LEADER)
        {
            *child = 3;
            top::discoverTopologyAsLeader(rank, clusterSerialized, clusterSerializedSize, cluster, topology);
        }
        else 
        {
            std::vector<int> clustersSerializedUnited;
            top::aggregateTopology(rank, parent, &clustersSerializedUnited, cluster);

            if (rank == crd::getLastCoordinator(ERROR_TYPE))
            {
                *child = -1;
                top::discoverTopologyAsLastCoordinator(rank, *parent, clustersSerializedUnited, topology, cluster);
            }
            else
            {
               top::discoverTopologyAsNormalCoordinator(rank, *parent, child, topology, coordNeighs, clustersSerializedUnited, cluster);
            }
        }
    }
    else /* ------------------------ FOLLOWER ------------------------ */
    {
        top::discoverTopologyAsFollower(rank, parent, topology);
        *child = -1;

        // Handle partition.
        if (*parent == 1 && ERROR_TYPE == crd::PARTITION_CONECTIVITY)
        {
            MPI_Finalize();
            return top::BELONGS_TO_PARTITION;
        }
    }

    return 0;
}