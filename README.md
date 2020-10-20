# How to Run ##
### Running: ###
- **IMPORTANT:** If running on RMIT servers, run `scl enable devtoolset-8 bash` first.
- Run: `make clean`
- Run: `make all`
- To run the program type: `/memory_allocator.o [file] [strategy] [output]`
  - `file` is the input file of information (names) to load in.
  - `strategy` refers to the allocation strategy and must be  `first`, `best` or `worst`.
  - `output` is the designated file to output to.
> e.g. /memory_allocator.o first-names-1.txt best best-output.txt

### Output: ###
After running an allocation strategy a file will be generated or overwritten in the root with the name specified at runtime (`output`).

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

```

Six files have already been generated in the `outputs` folder using the naming convention `[strategy]-[file]-output.txt`.
- `[strategy]` is the allocation stratergy and is either `first`, `best` or `worst`.
- `[file]` is the input file used, which is either `fnames` (`first-names-1.txt`) or `mnames` (`middle-names.txt`).