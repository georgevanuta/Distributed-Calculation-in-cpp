# Collaborative Calculation in Distributed Systems

`Score: 95/95`

## Table of Contents

- [Collaborative Calculation in Distributed Systems](#collaborative-calculation-in-distributed-systems)
  - [Table of Contents](#table-of-contents)
  - [Topology](#topology)
  - [Distributed Calculation](#distributed-calculation)
  - [Bonus](#bonus)

## Topology

I made a cluster datatype for easier data manipulation, which is:

```c++
typedef struct cluster_t
{
    int coordinator;
    std::set<int> otherMembers;

    // other methods...
} cluster_t;
```

Also, a datatype for the topology:

```c++
typedef std::map<int, std::vector<int>> topology_t;
```

which is a map from a coordinator to its followers.

In the first step, each coordinator will read its cluster from the input file:

```c++
const crd::cluster_t cluster = crd::readCluster(rank);
```

Then, it will send a message containing its rank to each of its followers.

```c++
crd::announceFollowers(cluster);
```

After that, if the error type is **2** (partition) and the rank of the coordinator is **1** (the partitioned cluster), it will
print its cluster as the known topology and then exit. Each of its members will do the same.

The algorithm I used for the communication between all processes is a "boomerang" algorithm. This means
that, in order for all processes to know everything about the topology, the leader (**0**) whill start communicating
with a children (**3**) which will do the same thing until the current process is the last one in our topology (either **1** or **2**).
After that, the communication will be reversed, from the last process to the leader.
This way, all the processes will aggregate the topology in the first wave, and, in the second wave they will
receive the whole topology.

In order to send a map (the topology), I serialized it in the following format:

$<TOPOLOGY>\ ::=\ <CLUSTER>|<CLUSTER><DELIM><TOPOLOGY>$
$<CLUSTER>\ ::=\ <COORDINATOR><FOLLOWERS>$
$<COORDINATOR>\ ::=\ unsigned\ int$
$<FOLLOWERS>\ ::=\ (unsigned\ int)^*$
$<DELIM>\ ::=\ -1$

## Distributed Calculation

The calculation works almost the same as the discovery of the topology.

The leader generates the vector. It will share a slice of the vector to its workers (which is directly proportional to
the number of workers) and then forward the rest of the vector to its child. Each coordinator, except for the last
one will do the same thing. The last one will not slice the vector, as it has received the last part of it
(which is already sliced). It will slice it and share the parts to its workers, and then forward the result
to its parent. Each coordinator, exept the leader, will forward it, concatanating its result. In the end,
the leader will receive the whole vector and will aggregate it with the result of its own workers.

In the end, the leader will hold the whole transformed vector and will print the result to STDOUT.

Photo with the load of each worker for a size of 10000000:

- No partition:

![workload](https://user-images.githubusercontent.com/74255152/209938304-6a83501f-cdc3-4e82-9ce9-fd3f3e0b45cb.png)

command ran:

```shell
make;mpirun --np 12 --oversubscribe ./tema3 10000000 1 | grep -e '::::'
```

- With partition:

command ran:

```shell
make;mpirun --np 12 --oversubscribe ./tema3 10000000 2 | grep -e '::::'
```

![workload_partition](https://user-images.githubusercontent.com/74255152/209938627-376cfe9b-5876-4bd0-8436-c45ed76fbf54.png)

## Bonus

This implementation handles the partition case by changing who is the last coordinator which will
send the feedback wave in the discovery of the topology and in the distributed calculation.

In the case of no partition ($ERROR\ =\ 0\ or\ 1$), the last coordinator is 1 and the wave will travel as
follows:

$0\ \rightarrow\ 3\ \rightarrow\ 2\ \rightarrow\ 1$

and then the feedback wave:

$1\ \rightarrow\ 2\ \rightarrow\ 3\ \rightarrow\ 0$

In the case of a partition ($ERROR\ =\ 2$), the last coordinator is 2 and the wave will travel as follows:

$0\ \rightarrow\ 3\ \rightarrow\ 2$

and the the feedback wave:

$2\ \rightarrow\ 3\ \rightarrow\ 0$
