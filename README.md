# Table of Contents

1. [Introduction] (README.md#introduction)
2. [Running Instructions] (README.md#running-instructions)
3. [Expected Output] (README.md#expected-output)
4. [Implementation Strategy] (README.md#implementation-strategy)
5. [Code Structure] (README.md#code-structure)
6. [Limitations] (README.md#limitations)
7. [Contact] (README.md#contact)

##Introduction

[Back to Table of Contents] (README.md#table-of-contents)

This has been a fun challenge with enough complexity for many small design decisions that enabled me to revisit a range of basic programming questions. I have avoided using libraries as I would use in a production environment (such as for json parsing and graph analysis) to facilitate evaluation of my programming.

To reduce memory use, I have decided to hold each name string in memory only once and store relationships between nodes by reference only. Transaction times are not stored but implied by relation to a variable holding the current most recent time and position in the first level hash table indexed by time seconds, which holds second level hash tables storing node pairs.

As large data sets were explicitly mentioned, I have decided to write in C++11 using the GNU g++ compiler version >= 4.7, for execution speed, flexibility, and convenience, as well as control over memory consumption and copy operations.

##Running Instructions

[Back to Table of Contents] (README.md#table-of-contents)

I am using the GNU GCC C++ compiler `g++` with C++11 support enabled, as specified in my `./src/Makefile`. The code can be searched for "C++11" and "GNU" to find depending statements, for which I have provided alternative implementation (GNU extension strnlen) or point to alternatives in the code comments.

The `run.sh` script changes directories into the `src` subdirectory and calls `make` there. It then changes back to the main project directory and calls the executable `./src/rolling_median`.

##Expected Output

[Back to Table of Contents] (README.md#table-of-contents)

For a discrete probability distribution, a median falling between two integers is defined as any number beween, and not equal to, these integers.
[See, for example https://en.wikipedia.org/wiki/Median#Probability_distributions](https://en.wikipedia.org/wiki/Median#Probability_distributions)

Example: In the case of node degrees (1, 1, 2, 2) my code output "1.50" as discussed in the project readme.

In the case where the median falls between numbers further than 1 apart, my code also outputs the smaller of the two numbers + 0.5, which is one of the correct medians.

Example: In the case of node degrees (1, 1, 3, 3, 3, 3, 5, 5, 5, 5, 9, 9) my code outputs "3.50" which is one of the correct values for the median, along with "4.00", "4.50" or any other number between 3 and 5.

I could add some logic to make the output in such cases "4.00" but it would add unnecessary complexity to my solution.

##Implementation Strategy

[Back to Table of Contents] (README.md#table-of-contents)

I read the input as single lines containing Json pairs of "name": "content" with any ordering of the three components for "actor", "target", and "created_time". I treat the input as medium-level hostile, i.e. I load the input into a limited size buffer to prevent buffer overflows. I require curly braces and quotation marks and exactly three sets of data, however I do not further sanitize the strings found, as they are only used for hash computation and string comparisons (matching in case of time). I check the time string for length before computing the epoch time and otherwise strings are evaluated as zero-terminated lists of bytes, so Bobby Tables-style injection attacks should not be successful on this code.

Evicting old edges and nodes from the graph, as well as inserting new edges connecting existing nodes requires looking up edges and nodes in the program's database. For fast execution, I have decided to implement edge and node lookup using hash lookup tables.

As transaction times are given only at full second accuracy, and only transactions from the 60 most recent seconds are to be maintained in the graph, the second after the minute of transactions forms a natural discrete hash for the edges.

Each bucket belonging to one of the time seconds holds another hash lookup table for the concatenated names of both nodes connected by the edges during that second. Non-directional edges are implemented by lexicographically ordering the name strings so that actor <= target while still in the input parser.

This two-level hash table approach allows faster eviction of entire hash tables for edges that have become obsolete ("aged out") when a more recent transaction arrives, by reducing the bookkeeping overhead from maintaining linked lists for hash collisions - since edges at different times are stored in different hash tables. This makes the small operation of inserting a new edge somewhat more expensive, as sixty hash tables have to be looked up to find if this edge already exists but makes the large operation of deleting entire sections of data that have aged out much cheaper. I believe this is a serviceable way of smoothing out computational effort for real-time application.

After actor and target are symmetrized by lexicographic ordering, all other code components treat actor and target as distinguishable and are capable of working with directional edges.

The bottleneck of the problem is lookup of nodes - does a new edge connect an existing node, and which node to update when deleting an edge? This is solved by a single level hash lookup table maintained for the node class, which holds the node's name and degree.

The median is computed from an array that is maintained to hold the occupancy of each degree. For example, if nodes of degree (1, 1, 2, 2) are present, the occupancy array reads (0, 2, 2) for zero nodes of degree zero, two nodes of degree one, and two nodes of degree two. The zeroth element of the array is never non-zero and was used for debugging. This array can grow to arbitrary size, allowing arbitrarily high degrees.

##Code Structure

I will refer to the files name.cpp and name.h as the `name` component of the code in the following.

Lines of input are read by the `venmoio` component into a data structure provided by the `venmodata` component. The `stringutils` component is used to parse and manipulate strings and assert Json syntax. The `epochtime` component is used to evaluate seconds since 1970 as common time base for comparing times of incoming transactions across minute, hour, day, month and year boundaries, as well as allow for leap seconds on certain days. Leap seconds are ignored in keeping with the problem posed by treating time differences exclusively.

The `hashtable` component implements a simple hash table structure with doubly linked lists for faster execution data eviction. the `hashtable` component provides the `htb::hash1` and `htb::mkhash2` functions outside the class structure for computing hashes of one and two strings for lookup in the node and edge hash tables, respectively. The range of the hash functions is controlled by the `htb::hashmask1` and `htb::hashmask2` bit masks, giving the size of their respective hash tables as (mask+1).

An abstract `Content` class is defined that all other data classes are derived from, so that multiple levels of hash tables and linked lists may contain each other.

The `List` and `Hashtable` classes provide some basic functionality for reading and writing references to data. Most importantly, `List::findBef` finds the last element of a list smaller or equal (lexicographically) to a passed `Content` item using the item's `compare` function, `Hashtable::evictListitem` evicts only one item from the table holding linked lists for hash collisions, and `Hashtable::insertListContent` inserts new `Content`.

The `graph` component defines the `Node` and `Edge` classes derived from `Content` and the `Graph` class governing the flow of data. The `Node` class holds a string for the name of the actor/target of a transaction, as well as the node's degree. Nodes are stored in the `Graph` member `ntab`, which is a hash table with the structure

- `ntab` hash table of size `htb::hashmask1 + 1` holding `Content*` references to linked lists of
  - `List` class holding `NULL` terminated `prev` and `next` pointers and a `content` reference to
    - `Node` holding the name sting and node degree

Edges are inserted into the `etab` hash table of `Graph`, which is indexed by seconds holding further hash tables with the `htb::mkhash2` function. This allows evicting whole seconds with minimal overhead. The data structure for edges is

- `etab` hash table with 60 elements holding references to
  - Hash tables labeled `sectab` in the code hlding `htb::hashmask2 + 1` linked lists
    - `List` class holding `NULL` terminated `prev` and `next` pointers and a `content` reference to
      - `Edge` class items, each holding two references to
        - `Node` class items holding their names and degrees.

The `graph` component provides implementations for the compare functions of the `Node` and `Edge` classes (lexicographically, actor and target ordered for non-directional edges) and data processing methods.

The `process` method receives a `venmodata` object and decides on how to treat it based on time. If it is newer than the newest existing data, the efficient `evictSectab` method is called to evict whole hashtables from the edge data, while keeping node data updated. If the new entry obsoletes all data, the efficient `evictAll` method wipes all data without having to compute any updates.

The new edge is inserted by creating an `Edge` object holding two new `Node` objects with degree 1 and calling the `insertEdge` method. Using the `evictEdge` method, the edge is looked up in the 60 hash tables (one for each second) in `etab` and an existing edge connecting the same nodes is evicted.

The new edge is inserted into its corresponding `sectab`. Upon attempting to insert an existing node, the `insertListContent` method of the List class deletes the new node and returns a reference to the existing node, which is detected and  existing node is inserted into the edge and updated instead (increased degree).

The `Graph` class holds a `degree` array for node degree occupancies, i.e. if there are four nodes of degree (1, 1, 2, 2), the `degree` array reads (0, 2, 2) for zero nodes of degree zero, two nodes of degree one, and two nodes of degree 2. A `maxdeg` member of the `Graph` class is maintained to facilitate inspection of the `degree` array. The `output` method sums up degree occupancies (effectively adding node numbers) and determines at which degree half the nodes are reached to output the median using only integer arithmetic.


##Limitations

[Back to Table of Contents] (README.md#table-of-contents)

There are a number of improvements I would make to the code that weren't within the scope of the limited time given.

Currently, leap seconds are treated "lazily", i.e. the Epoch time jumps back one second at 00:00:00 of the next day, just like the UNIX time standard.
This is easy to fix by adding a lookup table for leap seconds depending on calculated epoch time. However, since I treat eviction exclusively and the FAQ allows for inclusiveness, this is within the parameters of the problem.

My computed epoch time follows the UNIX time standard, counting seconds since 1970. I am using the C library time parsing utility which works for current times a couple of decades into the past and future. Updated future versions of the C library will expand this time period. I have included but not employed an alternative "eternal" calendar formula in the code. (epochtime::my_epochTime)

I have used standard C++ types throughout, without emplying long integer arithmetic. Given that the FAQ mentions that the code is to be run on a serial machine, I hope this does not pose a problem, as on 64 bit machines, the maximum unsigned integer is 4 billion and the maximum long long that I used to add up nodes is 9 10^18.

My code runs the longer of the provided input files (with about 1800 entries) in under one tenth of a second of user time on my old laptop, so it should be sufficient to pass the test's speed requirement. That said, when deleting whole database sections such as those aged out, iterating through hash tables could be made more efficient by maintaining a list of elements occupied, rather than testing table cells for `NULL` pointers. However, the code's bottleneck is the node table, which is accessed from the edge table for specific existing nodes via hashes only, unless the entire database is deleted. Nodes may store their hashes in a future version but they are cheap to recompute. A complete database wipe triggered by a transaction time at least one minute into the future benefits from reduced bookkeeping overhead, so this is not a serious limitation for runtime.


##Contact

Please contact me at andreas.markmann.phd@gmail.com
