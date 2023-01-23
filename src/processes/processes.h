#ifndef PROCESSES_H
#define PROCESSES_H

#include "../topology/topology.h"

#include <map>
#include <vector>


// Contains general functions that can be used by any process.
namespace proc
{
    // Logs a message to STDOUT in the following format: M(<fromProc>,<toProc>).
    void logMessage(const int fromProc, const int toProc);
    
    
    // Deserializes a topology in order to better access its members.
    top::topology_t deserializeTopology(int* serializedTopology, size_t topologySize);
    

    // Logs a topology to STDOUT in the following format: <CURRENT RANK> -> <CLUSTERS>
    void logTopology(const int rank, const top::topology_t topology);
}

#endif 