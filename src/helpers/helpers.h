#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <set>
#include <iostream>
#include <vector>
#include <algorithm>

#include "../coordinators/coordinators.h"


// Functions used for general stuff or for debugging.
namespace helper 
{

    
    // Exits if condition is true, printing the message to stdout.
    inline void exitIf(const bool condition, const std::string message) 
    {
        if (condition) {
            printf("%s", message.c_str());
            exit(1);
        }
    }

    
    /*
                --Used only for debugging--
        
            STDOUT delimiter.
    */    
    const std::string DELIM = "-----------------------------\n";
    

    /*
                --Used only for debugging--
        
            Prints the cluster of a coordinator
        in a nice format so that it doesn't intersect
        with other prints.
    */
    inline void printCluster(const crd::cluster_t cluster) 
    {
        std::string clusterOutput = DELIM + "COORDINATOR: " + std::to_string(cluster.coordinator) + "\n";

        for (auto &rank : cluster.otherMembers) {
            clusterOutput = clusterOutput + "PR: " + std::to_string(rank) + "\n";        
        }

        clusterOutput += DELIM;
        
        std::cout << clusterOutput;
    }


    /*
                --Used only for debugging--
        
            Prints the coordinator of a process in a 
        nice format so that it doesn't intersect with
        other prints.
    */
    inline void printCoordinator(const int rank, const int coordinator)
    {
        std::string coordOutput = 
            DELIM + "RANK: " + std::to_string(rank) + "\nCOORD: " + std::to_string(coordinator) + "\n" + DELIM;

        std::cout << coordOutput;
    }


    /*
                --Used only for debugging--
        
            Prints the rank, the child and the parent
        of a process in a nice format.
    */    
    inline void printFamily(const int rank, const int child, const int parent)
    {
        std::string familyOutput = 
            DELIM + 
            "RANK: " + std::to_string(rank) + 
            ", CHILD: " + std::to_string(child) + 
            ", PARENT: " + std::to_string(parent) + 
            "\n" + DELIM;
        
        std::cout << familyOutput;
    }
}

#endif