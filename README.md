# Table of Contents

1. [Introduction] (README.md#introduction)
2. [Running Instructions] (README.md#running-instructions)
3. [Implementation Strategy] (README.md#implementation-strategy)
4. [Input Loop Example] (README.md#input-loop-example)
5. [Limitations] (README.md#limitations)
6. [Contact] (README.md#contact)

##Introduction

[Back to Table of Contents] (README.md#table-of-contents)

This has been a fun challenge that enabled me to revisit a range of basic programming skills. I have avoided using libraries as I would use in a production environment (such as for json parsing and graph analysis) to facilitate evaluation of my programming skills.

To reduce memory use, I have decided to hold each name string in memory only once and store relationships between nodes by reference only. Transaction times are not stored but implied by relation to current most recent time and position in the first level hash table holding second level hash tables of node pairs.

As large data sets were explicitly mentioned, I have decided to write in C++11 using the GNU g++ compiler version >= 4.7, for execution speed, flexibility, and convenience, as well as control over memory consumption and copy operations.

##Running Instructions

[Back to Table of Contents] (README.md#table-of-contents)

I am using the GNU GCC C++ compiler g++ with C++11 support enabled, as specified in my Makefile. The "run.sh" script changes directories into the "src" subdirectories and calls "make" there. It then changes back to the main project directory and calls the executable "./src/rolling_median".

##Implementation Strategy

[Back to Table of Contents] (README.md#table-of-contents)

Evicting old edges and nodes from the graph, as well as inserting new edges connecting existing nodes requires looking up edges and nodes. For fast execution, I have decided to implement edge and node lookup using hash lookup tables.

As transaction times are given only at full second accuracy, and only transactions from the 60 most recent seconds are to be maintained in the graph, the second after the minute of transactions forms a natural discrete hash for the edges.

Each bucket belonging to one of the time seconds holds another hash lookup table for both nodes connected by the edges during that second. Non-directional edges are encoded by lexicographically sorting the name strings so that actor <= target while still in the input parser.

All other code components treat actor and target as distinguishable and are capable of working with directional edges, specifically edge hashes are computed from the concatenation of actor and target names.

A separate hash lookup table is maintained for the nodes, using a single name's hash and holding the node's degree.

An array of degrees holding the number of nodes having each degree is maintained and updated upon deletion and addition of edges. Using this array, the median degree is computed.

##Input Loop Example

The most recent time read from the input is designated as current time. Edges are maintained as a rolling array without reordering. If, say, the current time ends on 20 seconds, then

- edges held by the 20 second bucket are 0 seconds old,
- edges held by the 19 second bucket are 1 second old,
- edges held by the 18 second bucket are 2 seconds old,

and so on, until

- edges held by the 0 second bucket are 20 seconds old,
- edges held by the 59 second bucket are 21 seconds old,
- edges held by the 58 second bucket are 22 seconds old,

and so on, until

- edges held by the 21 second bucket are 59 seconds old.

If the data from the next line of input falls within the last 60 seconds, it is inserted or updated. If it is later, say by 10 seconds (i.e. falls on 30 seconds after the same minute as the previous time), then

- all edges held by buckets 21 to 30 are evicted, along with any nodes that are only connected by them, as they are now 60 to 69 seconds old,
- the new time ending on 30 seconds is designated as new current time,
- the new edge is inserted along with any new nodes they may connect.

##Limitations

[Back to Table of Contents] (README.md#table-of-contents)

There are a number of improvements I would make to the code that weren't within the scope of the limited time given.

Currently, leap seconds are treated "lazily", i.e. the Epoch time jumps back one second at 00:00:00 of the next day, just like the UNIX time standard.
This is easy to fix by adding a lookup table for leap seconds depending on calculated epoch time. However, since I treat eviction exclusively and the FAQ allows for inclusiveness, this is within the parameters of the problem.

My computed epoch time follows the UNIX time standard, counting seconds since 1970. I am using the C library time parsing utility which works for current times a couple of decades into the past and future. Updated future versions of the C library will expand this time period.
I have included but not employed an alternative "eternal" calendar formula in the code. (epochtime::my_epochTime)

I have used standard C++ types throughout, without emplying long integer arithmetic. Given that the FAQ mentions that the code is to be run on a serial machine, I hope this does not pose a problem, as on 64 bit machines, the maximum unsigned integer is 4 billion and the maximum long long that I used to add up nodes is 9 10^18.

Iterating through hash tables could be made more efficient by maintaining a list of elements occupied rather than testing for NULL pointers. However, the bottleneck is the node table and that is accessed from the edge table for specific existing nodes via computed hashes, unless the entire database is deleted. Nodes could store their hashes but they are cheap to recompute.


##Contact

Please contact me at andreas.markmann.phd@gmail.com
