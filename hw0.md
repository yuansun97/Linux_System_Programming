HW0

Chapter 1

1.1 Standard out
```C
    write(STDOUT_FILENO, "Hi! My name is Yuan.\n", 21);
```

1.2 Draw a triangle
```C
void write_triangle(int n) {
    if (n <= 0) {
        return;
    }
    for (int i = 1; i <= n; i++) {
        for (int j = i; j; j--) {
            write(STDOUT_FILENO, "*", 1);
        }
        write(STDOUT_FILENO, "\n", 1);
    }
}
```
    
1.3 Write to file
```C
    char * pathname_1 = "hello_world.txt";
    int flags = O_CREAT | O_TRUNC | O_RDWR;
    mode_t mode = S_IRUSR | S_IWUSR;
    int fd_1 = open(pathname_1, flags, mode);
    if (fd_1 == -1) {
        perror("open failed");
        return 0;
    }
    write(fd_1, "Hello, World! -- by write\n", 26);
    close(fd_1);
```

1.4 Printf to file
```C
    close(1);
    char * pathname_2 = "hello_world_2.txt";
    int fd_2 = open(pathname_2, flags, mode);
    printf("Hollo, World! -- by printf\n");
    close(fd_2);
```

1.5 write() v.s printf()
```
    write() is a system call which is costly, and it is used for writing a sequence of bytes.
    printf() is a standard library function and it can write data in different formats. 
    The call of the function itself is fast but it will ultimately invoke write().
```


Chapter 2

2.1 How many bits are there in a byte?
```
	8 bits.
```

2.2 How many bytes are there in a char?
```
	1 byte
```

2.3 How many bytes the following are on your machine?
```
	int: 4
	double: 8
	float: 4
	long: 8
	long long: 8
```

2.4 On a machine with 8 byte integers: int data[8];
    If the address of data is 0x7fbd9d40, then what is the address of data+2?
```
	0x7fbd9d50
```

2.5 What is data[3] equivalent to in C?
```
    	*(data + 3)
```

2.6 Why does this segfault? char *ptr = "hello"; *ptr = 'J';
```
	Some part of the process memory is read-only such as the instructions to CPU and the constants like "hello" above, so "* ptr = 'J';" is not allowed.
```

2.7 What does sizeof("Hello\0World") return?
```
	12
```

2.8 What does strlen("Hello\0World") return?
```
	5
```

2.9 Give an example of X such that sizeof(X) is 3.
```
	char *X = "ab";
```

2.10 Give an example of Y such that sizeof(Y) might be 4 or 8 depending on the machine.
```
	int Y = 2;
```


Chapter 3

3.1 What are two ways to find the length of argv?
```C
	int len = 0;
	while (argv[len] != NULL) { len++; }
```
or
```C
	argc;
```

3.2 What does argv[0] represent?
```
	It presents the program name.
```

3.3 Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?
```
	Environment variables are stored (together with command line arguments) at the top of the process memory layout followed by the stack.
```

3.4 On a machine where pointers are 8 bytes, and with the following code: char *ptr = "Hello"; char array[] = "Hello";
What are the values of sizeof(ptr) and sizeof(array)? Why?
```
	sizeof(ptr) = 8
	sizeof(array) = 6
	ptr is a pointer, sizeof(ptr) returns the number of bytes a pointer takes on this 64-bit machine which is 8 byte.
	array is an array of chars, siezeof(array) returns the the number of chars in this array, that is 'H', 'e', 'l', 'l', 'o' and '\0' sum to 6.
```

3.5 What data structure manages the lifetime of automatic variables?
```
	Stack.
```


Chapter 4

4.1 If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?
```
	Heap. Using malloc.
	char * ptr = malloc(128);
	free(ptr);
```

4.2 What are the differences between heap and stack memory?
```
Locate at different memory layout; stack is contiguously growing down on the memory layout, is automatically allocated and deallocated; 
  heap need to be both manually allocated and manually deallocated and is allocated in random order. 
```

4.3 Are there other kinds of memory in a process?
```
	There are text segment, initialized data segment, and uninitialized data segmen besides stack and heap.
```

4.4 Fill in the blank: "In a good C program, for every malloc, there is a ___".
```
	free.
```

4.5 What is one reason malloc can fail?
```
	No enough heap memory left.
```

4.6 What are some differences between time() and ctime()?
```
	time() returns the time since 00:00 UTC, Jan, 1970 in seconds.
	ctime() accepts single parameter time_ptr and returns the string representing the localtime.
e.g
	time_t secondsSince1970 = time(NULL);
	char * ptr = time(&secondsSince1970);
```

4.7 What is wrong with this code snippet? free(ptr); free(ptr);
```
	Double free.
```

4.8 What is wrong with this code snippet? 
```C
free(ptr); 
printf("%s\n", ptr);
```
```
	Using deallocated pointer.
```

4.9 How can one avoid the previous two mistakes?
```
	Avoid dangling pointer. One malloc, one free and set the pointer to NULL.
```

4.10 Create a struct that represents a Person. Then make a typedef, so that struct Person can be replaced with a single word. 
    A person should contain the following information: their name (a string), their age (an integer), 
    and a list of their friends (stored as a pointer to an array of pointers to Persons).
```C
	typedef struct Person person_t;
	struct Person {
		char * name;
		int age;
		person_t ** friends;
		int friends_count;
	};
```

4.11 Now, make two persons on the heap, "Agent Smith" and "Sonny Moore", who are 128 and 256 years old respectively and are friends with each other.
```C
	int main() {
		person_t * ptr1 = (person_t *) malloc(sizeof(person_t));
		person_t * ptr2 = (person_t *) malloc(sizeof(person_t));

		ptr1->name = "Agent Smith";
		ptr1->age = 128;
		ptr1->friends = &ptr2;

		ptr2->name = "Sonny Moore";
		ptr2->age = 256;
		ptr2->friends = &ptr1;

		free(ptr1);
		ptr1 = NULL;
		free(ptr2);
		ptr2 = NULL;

		return 0;
	}
```

4.12 create() should take a name and age. The name should be copied onto the heap. 
    Use malloc to reserve sufficient memory for everyone having up to ten friends. Be sure initialize all fields (why?). 
```C
	person_t * create(char * name_, int age_) {
    		person_t * ret = (person_t *) malloc(sizeof(person_t));
    		ret->name = strdup(name_);
    		ret->age = age_;
    		ret->friends = malloc(10 * sizeof(person_t *));
    		memset(ret->friends, 0, 10 * sizeof(person_t *));
    		return ret;
	}
```
There might be garbage data on the allocated momery. Initialization cleans the memory.


4.13 destroy() should free up not only the memory of the person struct, but also free all of its attributes that are stored on the heap. 
    Destroying one person should not destroy any others.
```C
	void destroy(person_t * p) {
    		free(p->name);
    		memset(p->friends, 0, 10 * sizeof(person_t *));
    		free(p->friends);
    		memset(p, 0, sizeof(person_t));
    		free(p);
	}
```


Chapter 5

5.1 What functions can be used for getting characters from stdin and writing them to stdout?
```C	
	int c;
	getchar(c);
	putchar(c);
```

5.2 Name one issue with gets().
```
	gets() limits the buffer size so it overwrites other data when the input length is bigger than the given size.
```

5.3 Write code that parses the string "Hello 5 World" and initializes 3 variables to "Hello", 5, and "World".
```C
	char * str = "Hello 5 World";
    	char buff_1[6];
    	char buff_2[6];
    	int num;
    	int result = sscanf(str, "%5s %d %s", buff_1, &num, buff_2);
```
    
5.4 What does one need to define before including getline()?
```
	#define _GNU_SOURCE
```

5.5 Write a C program to print out the content of a file line-by-line using getline().
```C
	char * pathname = "hello_world.txt";
    	FILE * fp = fopen(pathname, "r");
    	char * input;
    	size_t size = 0;
    	ssize_t line_size;
    	while ( getline(&input, &size, fp) >= 0 ) {
        	printf("%s", input);
    	}
    	free(input);
```


Chapter 6

6.1 What compiler flag is used to generate a debug build?
```
	-g
```

6.2 You modify the Makefile to generate debug builds and type make again. Explain why this is insufficient to generate a new build.
```
	Because "make" will not have extra flags such as -DEBUG/-g. One should use "make debug" with debug sepcified in the makefile e.g "debug: CXXFLAGS += -DDEBUG -g".
```

6.3 Are tabs or spaces used to indent the commands after the rule in a Makefile?
```
	tabs.
```

6.4 What does git commit do? What's a sha in the context of git?
```
	git commit -- Record changes to the repository.
	SHA is a unique ID created when a commit happens, also called "hash", 
	used as a record of what and when changes were made and by who.
```

6.5 What does git log show you?
```
	Information about pervious commits occurred in a project including commit hash, author, date etc.
```

6.6 What does git status tell you and how would the contents of .gitignore change its output?
```
	git status -- Tells which changes have been staged and which files aren't tracked by Git. 
	All the files whose paths are listed in .gitignore will not appear in git status.
```

6.7 What does git push do? Why is it not just sufficient to commit with git commit -m 'fixed all bugs' ?
```
	git push -- Upload local repository content to a remote repository. 
	git commit only updates the chages with local repository without making any changes to the remote.
```

6.8 What does a non-fast-forward error git push reject mean? What is the most common way of dealing with this?
```
	It means another person has pushed to the same branch.
	This can be fixed by ferching and merging the changes made on the remote branch with the local changes.
```





