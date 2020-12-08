# Angrave's 2019 Acme CS 241 Exam Prep		
## A.K.A. Preparing for the Final Exam & Beyond CS 241... 

Some of the questions require research (wikibook; websearch; wikipedia). 
It is accepted to work together and discuss answers, but please make an honest attempt first! 
Be ready to discuss and clear confusions & misconceptions in your last discussion section.
The final will also include pthreads, fork-exec-wait questions and virtual memory address translation. 
Be awesome. Angrave.

## 1. C 


1.	What are the differences between a library call and a system call? Include an example of each.
```
Library call: functions within the program library, for example `write`.

System call: functions provided by the kernel, for example `printf`.

System calls are costly compared with library calls, but some library calls ultimately invoke system calls. 
E.g. "printf" will invoke "write".
```


2.	What is the `*` operator in C? What is the `&` operator? Give an example of each.
```
'*' can be pointer, dereference operation, or multiply.

'&' can be address of, bitwise and.
```
```C
int a = 10;     
int *ptr = &a;          // ptr is a pointer to integer a
int b = (*ptr) * 2;     // b is 20
```


3.	When is `strlen(s)` != `1+strlen(s+1)` ?
```
When s is starting with '\0', for example 
```
```c
char *s = "\0Hello\0";    
int a = strlen(s);              // a is 0
int b = 1 + strlen(s + 1);      // b is 1 + 5 = 6
```
```
where `s + 1` skips the first '\0' and counts on the characters untill the second '\0'.
```


4.	How are C strings represented in memory? What is the wrong with `malloc(strlen(s))` when copying strings?
```
C strings are represented by a sequence of characters ended with '\0'.
Thus, when copying strings, we need to allocate one more byte for the '\0' as a ending symbol.
```


5.	Implement a truncation function `void trunc(char*s,size_t max)` to ensure strings are not too long with the following edge cases.
```
if (length < max)
    strcmp(trunc(s, max), s) == 0
else if (s is NULL)
    trunc(s, max) == NULL
else
    strlen(trunc(s, max)) <= max
    // i.e. char s[]="abcdefgh; trunc(s,3); s == "abc". 
```
    ```c
    void trunc(char *s, size_t max) {
        if (s == null || strlen(s) <= max) 
            return;
        else 
            s[max] = '\0';
    }
    ```


6.	Complete the following function to create a deep-copy on the heap of the argv array. Set the result pointer to point to your array. The only library calls you may use are malloc and memcpy. You may not use strdup.

    `void duplicate(char **argv, char ***result);` 
```c
void duplicate(char **argv, char ***result) {
	if (argv == NULL) {
		result = NULL;
		return;
	} 
    
    int len = 0;
	while (argv[len] != NULL) { len++; }

	*result = (char **) malloc(sizeof(char *) * len + 1);
	for (int i = 0; i < len; i++) {
		(*result)[i] = (char *) malloc( sizeof(char) * (strlen(argv[i]) + 1) );
		memcpy( (*result)[i], argv[i], strlen(argv[i]) + 1 );
	}
	(*result)[len] = NULL;
}
```

7.	Write a program that reads a series of lines from `stdin` and prints them to `stdout` using `fgets` or `getline`. Your program should stop if a read error or end of file occurs. The last text line may not have a newline char.
```c
    #define _GNU_SOURCE
    int mian() {
	char * pathname = "hello_world.txt";
    	FILE * fp = fopen(pathname, "r");
    	char * input;
    	size_t size = 0;
    	ssize_t line_size;
    	while ( getline(&input, &size, fp) >= 0 ) {
        	printf("%s", input);
    	}
    	free(input);
    }
```

## 2. Memory 

1.	Explain how a virtual address is converted into a physical address using a multi-level page table. You may use a concrete example e.g. a 64bit machine with 4KB pages. 
```
For a two-level indirection, 21 bits for the each level and the rest 12 bits are used for the page offset.
```

2.	Explain Knuth's and the Buddy allocation scheme. Discuss internal & external Fragmentation.
```
Buddy allocation scheme divides memory into partitions to try to satisfy a memory request as suitably as possible. 
This system makes use of splitting memory into halves to try to give a best fit. 
```

3.	What is the difference between the MMU and TLB? What is the purpose of each?
```
MMU: Memory Management Unit. Maps the virtual memory to physical memory.

TLB: Translation Lookaside Buffer. Used as a cache storing previously used page base addresses.

```

4.	Assuming 4KB page tables what is the page number and offset for virtual address 0x12345678  ?
```
Page number is 0x12345 and offset is 0x678.
```

5.	What is a page fault? When is it an error? When is it not an error?
```
When the process accesses an address that is not paged into the RAM a page fault occurs, or the process tries to access an invalid address. 
The second one is an error.
```

6.	What is Spatial and Temporal Locality? Swapping? Swap file? Demand Paging?
```
Spatial locality: if some data is accessed by the process, the neighboring space is likely to be accessed again.
Temporal locality: if some data is accessed by the process, the same data is likely to be accessed again.
```

## 3. Processes and Threads 

1.	What resources are shared between threads in the same process?
```
File handler, file descriptor table, variables outside the thread.
```

2.	Explain the operating system actions required to perform a process context switch
```
Store the state of the process; change the virtual memory space.
```

3.	Explain the actions required to perform a thread context switch to a thread in the same process
```
Store the state of the old thread.
```

4.	How can a process be orphaned? What does the process do about it?
```
An orphaned process is a running process whose parent has finished or terminated.
Orphaned processes will be taken care by the `init` process.
```

5.	How do you create a process zombie?
```
Never wait for the child process after fork.
```

6.	Under what conditions will a multi-threaded process exit? (List at least 4)
```
Signaled, the main thread finished, exited, canceled.
```

## 4. Scheduling 
1.	Define arrival time, pre-emption, turnaround time, waiting time and response time in the context of scheduling algorithms. What is starvation?  Which scheduling policies have the possibility of resulting in starvation?
```
arrival time: the time a process first arrives at the ready queue.
turnaround time: the total time from a process arrives at the ready queue to it ends.
waiting time: the sum of the time during which the process is not running, wait_time = (end_time - arrival_time) - run_time
response time: the time from a process arrives at the ready queue to it when the CPU actually starts working on it.

pre-emption: the existing process is removed before it finished when a more preferable process arrives at the ready queue.
starvation: 

SJF suffers from starvation.
```

2.	Which scheduling algorithm results the smallest average wait time?
```
Shortest-job-first (SJF) will minimize the total wait time across all jobs.
```

3.	What scheduling algorithm has the longest average response time? shortest total wait time?
```
Longest-job-first (LJF).
Shortest-job-first (SJF) has the shortest total wait time.
```

4.	Describe Round-Robin scheduling and its performance advantages and disadvantages.
```
Ad: ensures some notion of fairness
Dis: Large number of processes = Lots of switching
```

5.	Describe the First Come First Serve (FCFS) scheduling algorithm. Explain how it leads to the convoy effect. 
```
Processes are scheduled in the order of arrival.  
Long running processes could block all other processes.
```

6.	Describe the Pre-emptive and Non-preemptive SJF scheduling algorithms. 
```
Preemptive shortest job first is like shortest job first but if a new job comes in with a shorter runtime than the total runtime of the current job, it is run instead. 
```

7.	How does the length of the time quantum affect Round-Robin scheduling? What is the problem if the quantum is too small? In the limit of large time slices Round Robin is identical to _____?
```
Too small time slices cause too many swithcing.
Too large time slices -> FCFS
```

8.	What reasons might cause a scheduler switch a process from the running to the ready state?
```
A more preferable process arrives.
```

## 5. Synchronization and Deadlock

1.	Define circular wait, mutual exclusion, hold and wait, and no-preemption. How are these related to deadlock?
```
circular wait: continuously check a condition in a loop.
mutual exclusion: accessed by only one process at a time.
hold: access the data and block other processes
wait: wait for other process finish with the data

deadlock: some processes keep waiting for resources needed to go on, but every recource is occupied by some of themselves, causing the whole group of processes blocked. 
```

2.	What problem does the Banker's Algorithm solve?
```
^ deadlock
```

3.	What is the difference between Deadlock Prevention, Deadlock Detection and Deadlock Avoidance?
```
Deadlock prevention: ensures that at least one of the necessary conditions to cause a deadlock will never occur.

Deadlock detection: tell when a deadlock occurs

avoidance: ensures that the system will not enter an unsafe state.
```

4.	Sketch how to use condition-variable based barrier to ensure your main game loop does not start until the audio and graphic threads have initialized the hardware and are ready.
```
Call barrier wait after the creating audio thread and graphic thread.
```

5.	Implement a producer-consumer fixed sized array using condition variables and mutex lock.
```c
pthread_cond_t cv;
pthread_mutex_t m;
int count;

// Initialize
pthread_cond_init(&cv, NULL);
pthread_mutex_init(&m, NULL);
count = 0;

pthread_mutex_lock(&m);
while (count < 10) {
    pthread_cond_wait(&cv, &m); 
}
pthread_mutex_unlock(&m);

while (1) {
    pthread_mutex_lock(&m);
    count++;
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&m);
}
```

6.	Create an incorrect solution to the CSP for 2 processes that breaks: i) Mutual exclusion. ii) Bounded wait.
```
pthread_mutex_lock(p_mutex_t *m) {
  while(m->lock) ;
  m->lock = 1;
}
pthread_mutex_unlock(p_mutex_t *m) {
  m->lock = 0;
}
```

7.	Create a reader-writer implementation that suffers from a subtle problem. Explain your subtle bug.
```c
reader() {
    lock(&m)
    while (writers)
        cond_wait(&turn, &m)
    // No need to wait while(writing here) because we can only exit the above loop
    // when writing is zero
    reading++
    unlock(&m)

  // perform reading here

    lock(&m)
    reading--
    cond_broadcast(&turn)
    unlock(&m)
}

writer() {
    lock(&m)  
    writers++  
    while (reading || writing)   
        cond_wait(&turn, &m)  
    writing++  
    unlock(&m)  
    // perform writing here  
    lock(&m)  
    writing--  
    writers--  
    cond_broadcast(&turn)  
    unlock(&m)  
}
```

## 6. IPC and signals

1.	Write brief code to redirect future standard output to a file.
```c
close(1);
int fd = open("output.txt", O_RDWR | O_CREAT);
```

2.	Write a brief code example that uses dup2 and fork to redirect a child process output to a pipe
```

```

3.	Give an example of kernel generated signal. List 2 calls that can a process can use to generate a SIGUSR1.
```

```

4.	What signals can be caught or ignored?
```

```

5.	What signals cannot be caught? What is signal disposition?
```

```

6.	Write code that uses sigaction and a signal set to create a SIGALRM handler.
```

```

7.	Why is it unsafe to call printf, and malloc inside a signal handler?
```

```

## 7. Networking 

1.	Explain the purpose of `socket`, `bind`, `listen`, and `accept` functions
```

```

2.	Write brief (single-threaded) code using `getaddrinfo` to create a UDP IPv4 server. Your server should print the contents of the packet or stream to standard out until an exclamation point "!" is read.
```

```

3.	Write brief (single-threaded) code using `getaddrinfo` to create a TCP IPv4 server. Your server should print the contents of the packet or stream to standard out until an exclamation point "!" is read.
```

```

4.	Explain the main differences between using `select` and `epoll`. What are edge- and level-triggered epoll modes?
```

```

5.	Describe the services provided by TCP but not UDP. 
```

```

6.	How does TCP connection establishment work? And how does it affect latency in HTTP1.0 vs HTTP1.1?
```

```

7.	Wrap a version of read in a loop to read up to 16KB into a buffer from a pipe or socket. Handle restarts (`EINTR`), and socket-closed events (return 0).
```

```

8.	How is Domain Name System (DNS) related to IP and UDP? When does host resolution not cause traffic?
```

```

9.	What is NAT and where and why is it used? 
```

```

## 8. Files 

1.	Write code that uses `fseek`, `ftell`, `read` and `write` to copy the second half of the contents of a file to a `pipe`.
```

```

2.	Write code that uses `open`, `fstat`, `mmap` to print in reverse the contents of a file to `stderr`.
```

```

3.	Write brief code to create a symbolic link and hard link to the file /etc/password
```

```

4.	"Creating a symlink in my home directory to the file /secret.txt succeeds but creating a hard link fails" Why? 
```

```

5.	Briefly explain permission bits (including sticky and setuid bits) for files and directories.
```

```

6.	Write brief code to create a function that returns true (1) only if a path is a directory.
```

```

7.	Write brief code to recursive search user's home directory and sub-directories (use `getenv`) for a file named "xkcd-functional.png' If the file is found, print the full path to stdout.
```

```

8.	The file 'installmeplz' can't be run (it's owned by root and is not executable). Explain how to use sudo, chown and chmod shell commands, to change the ownership to you and ensure that it is executable.
```

```

## 9. File system 
Assume 10 direct blocks, a pointer to an indirect block, double-indirect, and triple indirect block, and block size 4KB.

1.	A file uses 10 direct blocks, a completely full indirect block and one double-indirect block. The latter has just one entry to a half-full indirect block. How many disk blocks does the file use, including its content, and all indirect, double-indirect blocks, but not the inode itself? A sketch would be useful.
```

```

2.	How many i-node reads are required to fetch the file access time at /var/log/dmesg ? Assume the inode of (/) is cached in memory. Would your answer change if the file was created as a symbolic link? Hard link?
```

```

3.	What information is stored in an i-node?  What file system information is not? 
```

```

4.	Using a version of stat, write code to determine a file's size and return -1 if the file does not exist, return -2 if the file is a directory or -3 if it is a symbolic link.
```

```

5.	If an i-node based file uses 10 direct and n single-indirect blocks (1 <= n <= 1024), what is the smallest and largest that the file contents can be in bytes? You can leave your answer as an expression.
```

```

6.	When would `fstat(open(path,O_RDONLY),&s)` return different information in s than `lstat(path,&s)`?
```

```

## 10. "I know the answer to one exam question because I helped write it"

Create a hard but fair 'spot the lie/mistake' multiple choice or short-answer question. Ideally, 50% can get it correct.
