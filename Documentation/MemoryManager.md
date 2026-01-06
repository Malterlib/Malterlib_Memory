Memory Manager {#p_Malterlib_Memory_MemoryManager}
==============

The Malterlib memory manager overall design goal is to allow concurrent memory allocations with minimal interaction between threads. Space-wise it's designed to minimize overhead and by default keep worst case internal fragmentation below 12.5%.

Overall architecture
====================

The memory manager is optionally numa-aware and employs three allocation strategies:

\image html MemoryManager.svg

Numa arena
----------

Every thread can assign the memory manager a numa arena through TCMemoryManager::f_SetNumaNode. Combining this with setting the thread affinity to the same numa node can realize significant performance benefits.

All threads that have not set the numa node will use the default numa arena, which has the node ID of ENumaNode_Default (-1). When a new thread is created that thread will inherit the numa node from its parent thread.


Arena
-----

Small allocations below 2 MiB are serviced by an arena.

Ideally each thread allocates memory from it's own arena, but to save virtual memory space arenas are shared between threads when possible. This is accomplished by each thread caching the last used arena and prefering this arena when it tries to check out an arena before doing allocations.

On 32 bit architectures the number of arenas are limited to 8 by default. See [Usage considerations](#ArenaCheckout) below.

### Slabs

When memory is allocated in an arena it will always come from a slab. A slab is a fixed size block of aligned memory that is divided into sub slabs:

\image html MemoryManagerSlab.svg

The slab is aligned on a 16 MiB (64 MiB with 16 KiB pages) address. This is to allow quick constant time calculation of the slab header with pointer arithmetic from an allocation address.

The header of the slab is located at the end of the slab which allows maximum utilization of address space when several sub slabs are used for one allocation and it keeps the alignment correct.

To allow checking if the header is a slab header a random magic number xored with the header address is stored in the header. This also requires that the last page of all 16 MiB (64 MiB with 16 KiB pages) adresses be a valid address for all allocations allocated in the memory manager.

The smallest allocation sizes are 1, 2 and 4 bytes, above that all allocations are always aligned on at least 4 bytes.

Depending on the desired maximum internal fragmentation each power of two allocation size is split up into a power of two number of sub sizes. The default is to split it up into 8 sub sizes per power of two. This gives a maximum internal fragmentation of 11.11% and the following sequence:

|	Size	|	Slab type		|	Sub size	|	Allocations/sub slab	|
|	----:	|	:----	----:	|	----:	|	----:	|
|	32	|	Type	0	|	4096	|	128	|
|	36	|	Type	1	|	36864	|	1024	|
|	40	|	Type	2	|	20480	|	512	|
|	44	|	Type	3	|	45056	|	1024	|
|	48	|	Type	4	|	12288	|	256	|
|	52	|	Type	5	|	53248	|	1024	|
|	56	|	Type	6	|	28672	|	512	|
|	60	|	Type	7	|	61440	|	1024	|
|	64	|	Type	0	|	4096	|	64	|
|	72	|	Type	1	|	36864	|	512	|
|	80	|	Type	2	|	20480	|	256	|
|	88	|	Type	3	|	45056	|	512	|
|	96	|	Type	4	|	12288	|	128	|
|	104	|	Type	5	|	53248	|	512	|
|	112	|	Type	6	|	28672	|	256	|
|	120	|	Type	7	|	61440	|	512	|


For detailed slab allocation sizes and overhead info see \ref p_Malterlib_Memory_MemoryManagerSlabs "Slab types".

All allocations returned from the slab allocator are returned with the inherent alignment of the size requested.

Heap
----

Sizes between 2 MiB and 16 MiB (64 MiB with 16 KiB pages) are allocated with a heap allocator. This allocator keeps track of allocations with a data structure outside of the allocated blocks to preserve the alignment of returned blocks.

Blocks returned are aligned on at least 64 KiB.

Currently only one heap alloctor is used for each numa arena, so allocations above 2 MiB are not concurrent. This should not be a problem, but if it turns out to be there could be one heap for each arena on 64 bit architectures where virtual address space in not limited.

Allocations are served from chunks allocated from the fallback allocator. The chunks are always 32 MiB in size (configurable).

To allow the arena to check for the slab header at the end of each 16 MiB (64 MiB with 16 KiB pages) boundrary the pages there are always kept committed even if no block has been allocated there.


Fallback allocator
------------------

For huge allocations above 16 MiB (64 MiB with 16 KiB pages) the fallback allocator will be used. This is the same allocator used to allocate the slabs and chunks in the arenas and heaps.

The size at which this allocator is used has to at least as large as the slab size to allow the slab header check when freeing or checking the size of a block of memory.

Usage considerations
====================

Platform limitations
--------------------

On 32 bit arcitechtures the number of arenas are limited to 8 by default. This thus becomes the maximum concurrency of the memory manager. To change this limit you can use f_SetMaxArenas. On 64 bit architectures the number of arenas are unlimited by default.

This arena limitation is needed because each arena use at least 128 MiB of virtual address space even if only one allocation for each slab type is allocated (16 MiB per slab * 8 slab types).

For 8 arenas this becomes 1 GiB of memory which is 25% or 50% of the available virtual address space depending on operatingg system.

This also means that there should be only one memory manager instance per process, otherwise you will soon run out of virtual address space. The global memory manager in Malterlib is by default shared between all modules that use Malterlib to solve this limitation.

Allocations between threads
---------------------------

If you allocate memory on one thread and free it on another thread, that memory will be put on a queue to be freed. This means that the memory will not actually be freed until the queue is processed.

The queue is processed before the arena allocates a new sub slab, when the memory manager is checked in and also on the background cleanup thread.

Arena checkouts {#ArenaCheckout}
---------------

For maximum perfomance allocating and freeing memory the memory manager needs to be "checked out". Without this each allocation will perform two atomic operations and each free will perform one atomic operation.

There is however a tradeoff here because on 32 bit architectures you only have 8 arenas to check out, so have this in mind when you check out the memory manager.

Usually the main thread should always have the memory manager checked out, but you should do a temporary checkin before going into an idle wait to allow the background cleanup to run on the arena.

Background cleanups
-------------------

Background cleanups are by default run on a background thread for each numa arena. By default the cleanup is run every 10 seconds if cleanup is necessary and it collects caches and "garbage" that is more than 10 seconds old.

The things cleaned up are:
#### Heaps
* Fully free chunks in heaps are freed in fallback allocator
* Committed unused memory is decommitted on heap chunks

#### Arenas
* Messages are processed on arenas
* Fully free sub slabs are returned to free list
* Fully free slabs are put in the numa arena cache of free slabs
* Committed unused sub slabs are decommitted

#### Numa arenas
* Cached arena slabs are decommitted
* Cached arena slabs are freed in the fallback allocator (only later 10 seconds after decommit)

This deferred cleanup scheme can significantly increase performance as it allows the non-cleaned up things act as a cache, while still optimally decreasing application memory usage to a minimum over time.

