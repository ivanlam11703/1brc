## My Attempt at the One Billion Row Challenge

**This code makes use of POSIX system calls. Notably mmap to map the entire 1 billion row file into memory. This makes it run much faster.**
***This code will NOT compile on Windows!***

This solution should run cold in about 50 seconds. After warming up the cache, it takes about 30 seconds to run.

The one billion row test file I used was generated by the java test generator provided from the [original 1brc repository](https://github.com/gunnarmorling/1brc).

My solution implements a custom simple hash for each city name used for indexing into the map. The hash allows for quick indexing into the map and also for less collisions. For collisions, I used a linear probe to resolve the collision and insert new cities into the map. 

Compile the code by running ```make``` and execute using ```./1brc <args>```.

My solution does not implement threading. I was very confused with indexing and how to make sure I can properly terminate chunks of code to hand to worker threads, since each line is not guaranteed to have the same formatting. This may be something I will return to.