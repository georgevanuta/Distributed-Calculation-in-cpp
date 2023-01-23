#include "processes.h"

#include <iostream>
#include <map>
#include <vector>
#include <string>


void proc::logMessage(const int fromProc, const int toProc)
{
    printf("M(%d,%d)\n", fromProc, toProc);
}


/*
        A serialized topology is an array of ints which respects the
    following strucure: the first element or an element after the value -1
    is the coordinator and the following elements between the previously
    mentioned delimiters are the workers of the respective cluster.
*/
top::topology_t proc::deserializeTopology(int* serializedTopology, size_t topologySize)
{
    int currentCoordinator = -1;
    top::topology_t topology = {};

    for (size_t i = 0; i < topologySize; i++)
    {
        // found a delimiter
        if (serializedTopology[i] == -1)
        {
            // start a new cluster
            currentCoordinator = -1;
        }
        else
        {
            // after a delimiter is the coordinator
            if (currentCoordinator == -1)
            {
                currentCoordinator = serializedTopology[i];
            }
            else
            {
                // found the workers of the current cluster
                topology[currentCoordinator].push_back(serializedTopology[i]);
            }
        }

    }

    return topology;
}


void proc::logTopology(const int rank, const top::topology_t topology)
{
    std::string topologyStr = std::to_string(rank) + " ->";

    for (auto const& pr : topology)
    {
        const int currentCoordinator = pr.first;
        const std::vector<int> currentCluster = pr.second;


        topologyStr += " ";
        topologyStr += std::to_string(currentCoordinator);
        topologyStr += ":";


        for (size_t i = 0; i < currentCluster.size() - 1; i++)
        {
            topologyStr += std::to_string(currentCluster[i]);
            topologyStr += ",";
        }

        topologyStr += std::to_string(currentCluster[currentCluster.size() - 1]);

    }

    printf("%s\n", topologyStr.c_str());
}
