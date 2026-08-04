# 📑 Laboratory Work #1: Basic Data Types & Variable Swapping

> **Course:** Programming Fundamentals (OP)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Master fundamental C/C++ data types, variable declarations, memory layout, formatted input/output, and algorithms for variable swapping and conditional value ordering.

---

## 📐 Theoretical Background

Programming in C/C++ requires a strong understanding of primitive data types, memory allocation, and basic algorithmic control structures.

### Primitive Data Types in C/C++

| Data Type | Typical Size | Value Range / Description |
| :--- | :---: | :--- |
| `char` | 1 Byte (8 bits) | ASCII character / integer ($-128$ to $127$) |
| `int` | 4 Bytes (32 bits) | Signed integer ($-2,147,483,648$ to $2,147,483,647$) |
| `float` | 4 Bytes (32 bits) | Single-precision floating point ($\approx 6\text{--}7$ decimal digits) |
| `double` | 8 Bytes (64 bits) | Double-precision floating point ($\approx 15\text{--}17$ decimal digits) |

### Variable Swapping Algorithms

Swapping the values of two variables $a$ and $b$:

1. **Using a Temporary Variable:**

   ```c
   double temp = a;
   a = b;
   b = temp;
   ```

2. **Without Temporary Variable (Arithmetic Approach):**

   ```c
   a = a + b;
   b = a - b;
   a = a - b;
   ```

3. **Without Temporary Variable (Bitwise XOR for Integers):**

   ```c
   a ^= b;
   b ^= a;
   a ^= b;
   ```

### Conditional Ordering Logic

To arrange three numbers $(z_1, z_2, z_3)$ in descending order ($z_1 \ge z_2 \ge z_3$), conditional `if / else` comparisons compare pairs and swap out-of-order elements:

```c
if (z1 < z2) { temp = z1; z1 = z2; z2 = temp; }
if (z1 < z3) { temp = z1; z1 = z3; z3 = temp; }
if (z2 < z3) { temp = z2; z2 = z3; z3 = temp; }
```

---

## 💻 Code & Files

- **Source Code:** [`main.c`](./main.c)
- **Lab Report:** [UA DOCX](./Звіт_ЛР1_Базові_типи_даних_та_змінні.docx)

---

## 🚀 How to Run

1. Compile C source code:

   ```bash
   gcc -O2 main.c -o main
   ```

2. Execute binary:

   ```bash
   ./main
   ```
