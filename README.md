# Advanced Compilation Project

## `MPI` collective detection and validation at compilation

This project builds a `gcc` plugin, inserting a pass verifying that all the `MPI` collectives defined in the code are executed by every possible iteration of the algorithm.

## How to use it

Just make the shared object by calling `make` in the `src` folder, then use it as a regular plugin.
To launch the verification process on a function, the following pragma has been defined:

```
#pragma mpicoll check function
#pragma mpicoll check (function1, function2)
```
