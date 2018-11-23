# Map Reduce

In 2004, engineers at Google introduced a new paradigm for large-scale parallel data processing known as MapReduce (see the original paper [here](https://static.googleusercontent.com/media/research.google.com/en//archive/mapreduce-osdi04.pdf)). One key aspect of MapReduce is that it makes programming such tasks on large-scale clusters easy for developers; instead of worrying about how to manage parallelism, handle machine crashes, and many other complexities common within clusters of machines, the developer can instead just focus on writing little bits of code (described below) and the infrastructure handles the rest.

## Getting Started

e give you here the
[`mapreduce.h`](https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/concurrency-mapreduce/mapreduce.h)
header file that specifies exactly what you must build in your MapReduce library:

``
#ifndef __mapreduce_h__
#define __mapreduce_h__

// Different function pointer types used by MR
typedef char *(*Getter)(char *key, int partition_number);
typedef void (*Mapper)(char *file_name);
typedef void (*Reducer)(char *key, Getter get_func, int partition_number);
typedef unsigned long (*Partitioner)(char *key, int num_partitions);

// External functions: these are what you must define
void MR_Emit(char *key, char *value);

unsigned long MR_DefaultHashPartition(char *key, int num_partitions);

void MR_Run(int argc, char *argv[], 
        Mapper map, int num_mappers, 
        Reducer reduce, int num_reducers, 
        Partitioner partition);

#endif // __mapreduce_h__
``

The most important function is `MR_Run`, which takes the command line
parameters of a given program, a pointer to a Map function (type `Mapper`,
called `map`), the number of mapper threads your library should create
(`num_mappers`), a pointer to a Reduce function (type `Reducer`, called
`reduce`), the number of reducers (`num_reducers`), and finally, a pointer to
a Partition function (`partition`, described below).

Thus, when a user is writing a MapReduce computation with your library, they
will implement a Map function, implement a Reduce function, possibly implement
a Partition function, and then call `MR_Run()`. The infrastructure will then
create threads as appropriate and run the computation.

## Contributors

The list of [contributors](https://github.com/HeerAmbavi/Map-Reduce/contributors) who participated in this project.


## Acknowledgments

* Prof. Nipun Batra, IIT Gandhinagar for all the support!

