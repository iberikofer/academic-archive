# 📑 Laboratory Work #5: Multidimensional Arrays & Matrices

> **Course:** Programming Fundamentals (OP)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Study multidimensional arrays (2D matrices) in C/C++, row-major memory layout, nested loop iteration techniques, matrix diagonal operations, and array transformation algorithms.

---

## 📐 Theoretical Background

A **two-dimensional (2D) array** or matrix represents a tabular data structure consisting of $R$ rows and $C$ columns.

### Row-Major Memory Storage

In C/C++, 2D arrays are stored in contiguous memory in **row-major order** (elements of row 0 consecutively, followed by row 1, etc.):
$$\text{Address}(\text{matrix}[i][j]) = \text{BaseAddress} + (i \times C + j) \times \text{sizeof}(\text{DataType})$$

```cpp
int matrix[R][C]; // Declares a 2D array of R rows and C columns
```

### Matrix Diagonal Properties (Square Matrix $N \times N$)

For a square matrix $M_{N \times N}$:

- **Main Diagonal:** Consists of elements where row index equals column index ($i = j$):
  $$\text{MainDiagonal} = \{ M[i][i] \mid 0 \le i < N \}$$
- **Secondary (Anti-) Diagonal:** Consists of elements where $i + j = N - 1$:
  $$\text{SecondaryDiagonal} = \{ M[i][N - 1 - i] \mid 0 \le i < N \}$$
- **Above Main Diagonal:** Elements where $j > i$.
- **Below Main Diagonal:** Elements where $j < i$.

### Nested Loop Traversal Algorithm

To iterate over every element in a 2D matrix:

```cpp
for (int i = 0; i < R; i++) {
    for (int j = 0; j < C; j++) {
        // Access element matrix[i][j]
    }
}
```

---

## 💻 Code & Files

- **Source Code:** [`main.cpp`](./main.cpp)
- **Lab Report:** [UA DOCX](./Звіт_ЛР5_Багатовимірні_масиви_та_матриці.docx)

---

## 🚀 How to Run

1. Compile C++ source code:

   ```bash
   g++ -O2 main.cpp -o main
   ```

2. Execute binary:

   ```bash
   ./main
   ```
