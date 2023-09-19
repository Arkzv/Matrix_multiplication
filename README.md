# Parallel Matrix Multiplication with Python/C API

This code snippet demonstrates parallel matrix multiplication in Python using the C API while bypassing the Global Interpreter Lock (GIL).

## Overview:

   * **Data Structure:** The data structure is defined on the Python side as a 2D list, and the returned type is also a standard 2D list. The results are in double precision.

   * **Parallelization:** The computations are parallelized across the CPUs.

## Compilation (Linux):

To compile the C code, follow replace **python3.10** with your specific Python version if necessary.

    bash
    gcc -I/usr/include/python3.10 -shared -fPIC -O3 -o matrix_multiplication.so matrix_multiplication.c

## Usage:

The compiled library can be included and used in Python as usual.
Number of Threads:

You can specify the number of threads to be used for computation in the multiply_matrices function.

Example:

```python

import random as rd
import matrix_multiplication

# Define matrix dimensions
d1 = 50
d2 = 800

# Generate random matrices
matrix1 = [[rd.random() for _ in range(d2)] for _ in range(d1)] 
matrix2 = [[rd.random() for _ in range(d1)] for _ in range(d2)]

# Specify the number of threads
num_threads = 6

# Call the parallel_matrix_multiply function
result = matrix_multiplication.multiply_matrices(matrix1, matrix2, num_threads)
```
