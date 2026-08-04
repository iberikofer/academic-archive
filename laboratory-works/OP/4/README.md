# 📑 Laboratory Work #4: Functions & Parameters

> **Course:** Programming Fundamentals (OP)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Study modular programming concepts in C/C++, function prototypes, parameter passing mechanisms (by value, pointer, and reference), call stack frames, and implementation of custom recursive functions.

---

## 📐 Theoretical Background

A **function** is a self-contained block of statements designed to perform a specific subtask. Modular function decomposition improves code readability, maintainability, and code reusability.

### Function Prototype and Definition

- **Function Prototype (Declaration):** Informs the compiler about the function's signature before its usage:

  ```cpp
  double calculate_power(double base, int exponent);
  ```

- **Function Definition:** Contains the actual implementation:

  ```cpp
  double calculate_power(double base, int exponent) {
      double result = 1.0;
      for (int i = 0; i < exponent; i++) result *= base;
      return result;
  }
  ```

### Parameter Passing Mechanisms

1. **Pass-by-Value:** A local copy of the parameter is created in the function stack frame. Changes inside the function do not affect the caller's original variable:

   ```cpp
   void increment(int x) { x++; }
   ```

2. **Pass-by-Pointer (C/C++):** Passes the memory address of the argument, allowing direct modification of caller variables:

   ```cpp
   void swap_pointers(int *a, int *b) {
       int temp = *a;
       *a = *b;
       *b = temp;
   }
   ```

3. **Pass-by-Reference (C++):** Creates an alias for the passed argument:

   ```cpp
   void swap_refs(int &a, int &b) {
       int temp = a;
       a = b;
       b = temp;
   }
   ```

### Call Stack & Recursion

When a function is called, a **stack frame** containing function arguments, local variables, and return memory addresses is pushed onto the call stack. Recursive functions call themselves until a base termination condition is met:
$$F(n) = \begin{cases} 1, & \text{if } n \le 1 \\ n \times F(n-1), & \text{if } n > 1 \end{cases}$$

---

## 💻 Code & Files

- **Source Code:** [`main.cpp`](./main.cpp)
- **Lab Report:** [UA DOCX](./Звіт_ЛР4_Функції_та_параметри.docx)

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
