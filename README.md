## **Operating Systems**

**Note**

These are the assignments that my teammate and I completed in this course. That means the starter code of the assignments were given to us by the instructor and we worked on completing the required tasks, i.e. creating a `FUSE file system`, `synchronization in message queues`, and `virtual memory`. The description of each assignment can be viewed in a pdf in this repo.

As the starter code of the assignments of this course are the instructor's property, I have removed most of the implementations from the files. That means this public repo is an extremely *partial* view of the implementations. Please request to view the complete impelmentation on my private repo.


### **`Assignment 1`**

This assignment consists of 2 parts:

**part 1**

**File system proposal**: In this part we worked on a file system proposal in which we created an *extent* based file system. In this proposal we designed how/where different components of the file system should be. The components of our file system are:

- Super block

- Free extents table

- Inode bitmap

- Inode table 

- Data blocks bitmap


Our file system is called `a1fs`.

<kbd><img src="/A1/fileSystem.png" style="width:450px;height:300px;" align="center"></kbd>

**part 2**

**File system implementation**: In this part we implemented the actual file system. 		 	 	 		
					
We used `FUSE` to interact with the file system. `FUSE` allowed us to implement a file system in user space by implementing the callback functions that the library needs to call. The layers involved when using a file system code with `FUSE` can be viewed in the diagram above. As it is shown in the top right, `a1fs` has been executed to mount our file system at `/tmp/mnt`. Now when we run the program `ls  -l  /tmp/mnt`, the `ls` program will execute the system calls to list the contents of the directory. These system calls are executed in the kernel which recognizes that they need to be handled by the `FUSE` file system. `FUSE` then executes the callback functions that we have written including `a1fs_getattr()` and `a1fs_readdir()`.

**Running the `a1fs` file system**

1. Create a disk image file (to be like our disk storage), by:

```
truncate -s <size> <image file>
./mkfs.a1fs -i <number of inodes> <image file>

```

`truncate` will create the image file if it doesn't exist and will set its size. `mkfs.a1fs` will format it into the `a1fs` file system.					
				
The next step is to mount the file system. You will need to choose a location in the system's file tree to place the image file under that mount point. So run:

```
/a1fs <image file> <mount point>
```

The image file is the disk image formatted by `mkfs.a1fs`. Not only does `a1fs` mount the disk into the local file system, but it also sets up callbacks and then calls `fuse_main()` 
work. Both `a1fs` and `mkfs.a1fs` have additional options, run them with `-h` to see their descriptions.

After the file system is mounted, you can access it using standard tools. To `unmount` the file system, run:

```
fusermount -u <mount point>
```

Note that you should be able to unmount the file system after any sequence of operations, such that when it is mounted again, it has the same contents.
		

You can view the detailed completion requirements of this assignment [*here*](/A1/assignment1a_proposal.pdf) and [*here*](/A1/assignment1b_implementation.pdf).

---


### **`Assignment 2`**

In this assignment, we implemented a message queue which can be written to and read from by several other threads. The access to this message queue is controlled by the *synchronization primitives* such as `locks` and `condition variables`. The use case of this application is where we have several readers and writers that want to write/consume the messages in a queue. This application can be in a distributed architecture where writer threads can write received messages to a distributed, non-local queue and several other distributed reader threads then want to read those messages from the queue. The application of this assignments is all about managing such use case by using synchronization and polling/subscribing.


**part 1**

**Message queue implementation** 

For each single queue, we have a single backend but since several different readers or writers want to interact with the queue for different purposes, we let each of them get an integer handle of that queue to subscribe to that. Each integer handle (i.e `msg_queue_t`), is specific to that reader/writer as it has the required flags embedded into it.


<kbd><img src="/A2/message_queue.png" style="width:450px;height:300px;" align="center"></kbd>

**part 2**

**I/O multiplexing functionality on queues**

In this part we added an I/O multiplexing functionality on queues, by implementing a poll function that monitors for any event (i.e messages) on different messages queues.

<kbd><img src="/A2/poll.png" style="width:450px;height:300px;" align="center"></kbd>

**Note**: Each reader/writer thread will call `msg_queue_poll()` with its own list of queues that it wants to subscribe to. So all variables in this function (i.e conditional variables, lock, ...) are specific to that reader/writer thread.

**Note**: In the struct Node, we have a member called *entry* that is an entry to a linked list. We saved it since by knowing this we can access the pointer of this node by using the function `container_of` in `list.h`. So if a thread wants to subscribe to multiple queues, it will have one unique node (and so entry) for each of them.
 

You can view the detailed completion requirements of this assignment [*here*](/A2/assignment2_messageQueues.pdf.pdf).

---

### **`Assignment 3:`**

In this assignment, we investigated memory access patterns in different programs, simulated the operation of page tables and implemented several *page replacement algorithms*.
 
**Memory reference traces**

`Valgrind` has an option that allows us to print out the memory reference trace of a running program. The memory reference trace of 3 different programs (with different degrees of localities) between two parts of the code in each, was given to us. The memory traces are saved as files `pages-*.ref`.
We traced the memory references of these 3 types of programs. Then we investigated how the accesses of different pages of the memory are distributed in terms of the number of `Instruction/Load/Store/Modify` accesses. We outputted the results similar to below:
 
<kbd><img src="/A3/memory_trace.png" style="width:450px;height:300px;" align="center"></kbd>
 
We concluded that the programs that are written such that the locality in accessing the data is paid attention to, have significantly much less *page misses* and hence are more memory efficient and faster. For example `blocked matrix multiplication` vs `naive matrix multiplication`.
 
**Virtual to physical translation**
 
In this part, we implemented a simulation of the *virtual-to-physical address translation* and *demand paging* using a two-level page table. Then we implemented three different page replacement algorithms: `FIFO`, `Clock`, `exact LRU`. Then we analyzed some memory traces from part 1 to analyse the number of pages misses using different page replacement algorithms.
 
To run the simulated memory, run:

```
./sim -f <tracefile> -m <memory size> -s <swapfile size> -a <replacement algorithm>
```
 
`memory size` is the size of memory you want for example 16 MiB
`swapfile size` is the size of the swap file used when we evict the pages out from the simulated memory
`replacement algorithm` is the replacement algorithm used.
 
The physical memory is simulated by an `char array` which holds each physical frame and its assigned virtual address. On the other side, we have an array of frame structures that point to their assigned page table entries. Each page table entry has the frame number it has been assigned to and has also the `ref`, `valid`, `dirty`, and `swap` bits that are embedded into the frame integer. The information bits are as follows:
 
<kbd><img src="/A3/bits.png" style="width:450px;height:200px;" align="center"></kbd>
 
The following is the overview of the data structures used in this assignment and how different types of them correspond to each other.
 
<kbd><img src="/A3/page_table.png" style="width:450px;height:300px;" align="center"></kbd>
 
<kbd><img src="/A3/page_table_2_levels.png" style="width:450px;height:300px;" align="center"></kbd>
 
<kbd><img src="/A3/simulated_memory.png" style="width:450px;height:300px;" align="center"></kbd>
 
<kbd><img src="/A3/frame_array.png" style="width:450px;height:300px;" align="center"></kbd>
 
<kbd><img src="/A3/page_dir_array.png" style="width:450px;height:300px;" align="center"></kbd>
 
In general, this assignment simulated the whole procedure that happens when accessing a line of the code like `store/modify/load` form a virtual address. From accessing the page table and assigning a memory frame, to setting `ref` bits and updating the data structures used in the page replacement algorithm.
 
Finally, we implemented and investigated the performance of `FIFO`, `Clock`, `LRU` page replacement algorithms. For instance, we implemented the `LRU` using a doubly linked list as below:
 
<kbd><img src="/A3/LRU.png" style="width:450px;height:300px;" align="center"></kbd>


You can view the detailed completion requirements of this assignment [*here*](/A3/assignment3_virtualMemory.pdf) and our analysis of page replacement algorithms [*here*](/A3/analysis.pdf).

