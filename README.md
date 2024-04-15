# Buddy Heap Management System with Garbage Collection Simulations

### Introduction
This project implements a Buddy Heap Management System along with simulations of two garbage collection mechanisms: Reference Counting and Mark-Sweep. Additionally, it utilizes graphs to represent mutually referenced data nodes.

### Buddy Heap Management System
The Buddy Heap Management System partitions memory into blocks of sizes that are powers of 2. When a request for memory allocation is made, the system allocates memory from the appropriate sized block, splitting larger blocks if necessary. When memory is deallocated, the system merges adjacent free blocks to form larger blocks, following a buddy system where each block has a partner of equal size.

### Heap Compaction
Heap compaction is called on every memory deallocation to optimize memory usage by compacting/merging contiguous free memory blocks. This reduces fragmentation and improves memory allocation efficiency.



### Garbage Collection Simulations
1. **Reference Counting**:
   - This garbage collection mechanism keeps track of the number of references to each allocated object. When the reference count drops to zero, the object is considered garbage and is deallocated.
   
2. **Mark-Sweep**:
   - This mechanism traverses all allocated memory, marking each object as reachable or unreachable. Then, it sweeps through the memory, deallocating unreachable objects.
   - An optimized constant space traversal technique without recursion or stack is implemented for this method.

### Graph Representation
Graphs are utilized to represent mutually connected data nodes, facilitating efficient management of interconnected data structures.

### Implementation Details
- **Buddy Heap Management**: 
  - The system manages memory allocation and deallocation efficiently using a buddy system algorithm.
- **Reference Counting**:
  - Reference counts are maintained for each allocated object, and objects with zero references are deallocated.
- **Mark-Sweep**:
  - Implemented with a constant space traversal technique, avoiding recursion or stack overheads.
- **Graph Representation**:
  - Nodes and edges are used to represent interconnected data structures, aiding in visualizing relationships between pointers.
