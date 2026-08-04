# 📑 Laboratory Work #3: One-Dimensional Arrays & Search Algorithms

> **Course:** Programming Fundamentals (OP)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Study one-dimensional (1D) arrays in C/C++, memory layout, zero-based array indexing, linear search techniques, min/max element extraction, and element transformation algorithms.

---

## 📐 Theoretical Background

An **array** is a fixed-size, sequential collection of elements of the same data type stored in contiguous memory locations.

### Array Declaration & Memory Access

In C/C++, array elements are indexed from $0$ to $n-1$, where $n$ is the array size:

```cpp
int arr[10]; // Declares an array of 10 integers
```

The memory address of element `arr[i]` is computed directly as:
$$\text{Address}(\text{arr}[i]) = \text{BaseAddress}(\text{arr}) + i \times \text{sizeof}(\text{DataType})$$

### Linear Search & Extremum Algorithms

1. **Finding Minimum / Maximum Element:**

   ```cpp
   int min_val = arr[0];
   int min_idx = 0;
   for (int i = 1; i < n; i++) {
       if (arr[i] < min_val) {
           min_val = arr[i];
           min_idx = i;
       }
   }
   ```

2. **Linear Search:**
   Iterates through the array elements sequentially to locate a target key value:

   ```cpp
   int target_idx = -1;
   for (int i = 0; i < n; i++) {
       if (arr[i] == target_key) {
           target_idx = i;
           break;
       }
   }
   ```

   - **Time Complexity:** $\mathcal{O}(n)$
   - **Space Complexity:** $\mathcal{O}(1)$

---

## 💻 Code & Files

- **Source Code:** [`main.cpp`](./main.cpp)
- **Lab Report:** [UA DOCX](./Звіт_ЛР3_Одновимірні_масиви_та_пошук.docx)

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
