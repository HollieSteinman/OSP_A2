# How to Run ##
## Running: ##
- **IMPORTANT:** If running on RMIT servers, run `scl enable devtoolset-8 bash` first.
- Run: `make clean`
- Run: `make all`
- To run the program type: `/memory_allocator.o [file] [strategy]`
  - `file` is the input file of information (names) to load in.
  - `strategy` refers to the allocation strategy and must be  `first`, `best` or `worst`.
> e.g. /memory_allocator.o first-names-1.txt best

## Output: ##
After running an allocation strategy a file will be generated or overwritten in the root with the naming convention `[strategy]-output.txt`. Where `strategy` is either `first`, `best` or `worst`.

The output file is formatted as such:
```
Total memory allocated: X

-----freedMBList-----
[processid]     [size]
[processid]     [size]
...

-----allocMBList-----
[processid]     [size]
[processid]     [size]
...

````