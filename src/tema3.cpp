#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <map>

#include "coordinators/coordinators.h"
#include "processes/processes.h"
#include "helpers/helpers.h"
#include "topology/topology.h"
#include "leader/leader.h"
#include "calculate/calculate.h"


#define UNUSED(expr) (void)(expr)


int main(int argc, char *argv[]) 
{
    // --------MPI INIT-------- //
    int numProcs, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // --------VARS INIT-------- //
    fflush(stdout);
    helper::exitIf(argc != 3, "[USAGE]: mpirun --oversubscribe --np <P> ./tema3 <DIM> <ERROR_TYPE>\n");
    helper::exitIf(std::stoi(argv[1]) < 0, "[ERROR]: <DIM> should be a positive integer.\n");

    const size_t DIM = std::stoi(argv[1]);
    const unsigned int ERROR_TYPE = std::stoi(argv[2]);

    // ------------------- DISCOVER TOPOLOGY ------------------- //
    top::topology_t topology;
    int parent;
    int child;

    int ret;

    ret = top::discoverTopology(rank, ERROR_TYPE, &topology, &parent, &child);

    // If process is in a partition return early.
    if (ret == top::BELONGS_TO_PARTITION) return 0;


    // ------------------- CALCULATE VECTOR ------------------- //

    if (rank == crd::LEADER)
    {
        // generate the vector
        std::vector<int> vec = leader::generateVector(DIM);
        
        // hold a slice of it and forward the rest
        std::vector<int> slicedVec = calc::sliceAndShareVector(rank, child, 0, topology, vec);
        
        // distribute the workload equally to the workers
        calc::shareVectorToFollowers(rank, topology, slicedVec);
        
        // aggregate the results from the workers
        std::vector<int> workersRes = calc::aggregateFollowersVecs(rank, topology);
        
        // aggregate the result with the result of the child
        std::vector<int> aggregatedVec = calc::aggregateVectors(rank, child, workersRes);
        
        // print the final solution
        leader::printResult(aggregatedVec);
    }
    else if (crd::isCoordinator(rank))
    {
        if (rank == crd::getLastCoordinator(ERROR_TYPE))
        {
            // receive the last slice of the vector
            size_t accProcs;
            std::vector<int> slicedVec = calc::recvNextVector(rank, parent, &accProcs);
            
            // distribute it to your followers
            calc::shareVectorToFollowers(rank, topology, slicedVec);
            
            // aggregate the results
            std::vector<int> workersRes = calc::aggregateFollowersVecs(rank, topology);
            
            // as this is the last process in our topology, it will not aggregate
            // the results from the children, as it has none
            calc::shareTransformedVector(rank, parent, workersRes);
        }
        else
        {
            // receive a slice of the vector
            size_t accProcs;
            std::vector<int> vec = calc::recvNextVector(rank, parent, &accProcs);

            // forwars the rest to the child coordinator
            std::vector<int> slicedVec = calc::sliceAndShareVector(rank, child, accProcs, topology, vec);
            
            // equally distribute the vector to your followers
            calc::shareVectorToFollowers(rank, topology, slicedVec);
            
            // aggregate the results
            std::vector<int> workersRes = calc::aggregateFollowersVecs(rank, topology);
            std::vector<int> aggregatedVec = calc::aggregateVectors(rank, child, workersRes);

            // send the result to the parent
            calc::shareTransformedVector(rank, parent, aggregatedVec);
        }
    }
    else
    {
        // receive the slice of the vector from the cluster coordinator
        std::vector<int> vec = calc::recvVecAsFollower(parent);
        
        // UNCOMMENT THIS LINE IF YOU WANT TO SEE THE WORKLOAD OF EACH WORKER
        calc::printVec(rank, vec);

        // multiply it
        std::vector<int> vecTransformed = calc::transformVector(vec);
        
        // send it back to the coordinator
        calc::sendVecAsFollower(vecTransformed, rank, parent);
    }


    MPI_Finalize();
}