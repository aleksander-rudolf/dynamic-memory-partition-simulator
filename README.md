# Dynamic Partition Simulator

This is a worst-fit dynamic partition memory allocation simulator that approximates some of the functionality of malloc() and free() in the standard C library. The input to the simulator will be a page size (a positive integer) and list of allocation and deallocation requests. The simulator will
simulate processing all requests and then compute some statistics. Throughout the simulation the program will maintain an ordered list of dynamic partitions. Some
partitions will be marked as occupied, the rest will be marked as free. Occupied partitions will have a numeric tag attached to it. Each partition will also contain its size in bytes, and the starting address. The starting address of the first partition should be 0. The simulator will manipulate this list of partitions as
a result of processing requests. Allocation requests will be processed by finding the most appropriately sized partition and then allocating a memory from it. Deallocation requests will free up any relevant occupied partitions, and also merging any adjacent free partitions.

To compile all code, type:
```
$ make
```

---
# Test files:
```
$ ./memsim 123 < test1.txt
pages requested:                58
largest free partition size:    129
largest free partition address: 221
elapsed time:                   0.001

$ ./memsim 321 < test2.txt
pages requested:                16
largest free partition size:    136
largest free partition address: 5000
elapsed time:                   0.000

$ ./memsim 111 < test3.txt
pages requested:                0
largest free partition size:    0
largest free partition address: 0
elapsed time:                   0.000

$ ./memsim 222 < test4.txt
pages requested:                896
largest free partition size:    995
largest free partition address: 5
elapsed time:                   0.005

$ ./memsim 333 < test5.txt
pages requested:                141824
largest free partition size:    11707
largest free partition address: 29781916
elapsed time:                   0.571

$ ./memsim 606 < test6.txt
pages requested:                3558653
largest free partition size:    8807
largest free partition address: 857672560
elapsed time:                   1.483

$ ./memsim 100000 < test7.txt
pages requested:                1
largest free partition size:    99894
largest free partition address: 106
elapsed time:                   0.000
```
