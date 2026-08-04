# 📑 Laboratory Work #2: Control Flow & Loops

> **Course:** Programming Fundamentals (OP)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Master control flow constructs in C/C++, implement iterative loop structures (`for`, `while`, `do-while`), understand loop termination criteria, and analyze brute-force / function tabulation algorithms.

---

## 📐 Theoretical Background

Loop constructs allow executing a block of code repeatedly while a specific condition holds true.

### Loop Types in C/C++

1. **`for` Loop (Count-Controlled Loop):**
   Used when the number of iterations is known in advance:

   ```cpp
   for (initialization; condition; increment/decrement) {
       // Loop body
   }
   ```

2. **`while` Loop (Pre-Test Loop):**
   Evaluates the condition *before* executing the loop body:

   ```cpp
   while (condition) {
       // Loop body
   }
   ```

3. **`do-while` Loop (Post-Test Loop):**
   Executes the loop body *first*, then checks the condition (guarantees at least 1 execution):

   ```cpp
   do {
       // Loop body
   } while (condition);
   ```

### Loop Control Flow Statements

- `break`: Immediately exits the innermost loop or `switch` block.
- `continue`: Skips the remainder of the current loop body iteration and proceeds to the next iteration test.

### Function Tabulation Algorithm

To tabulate a function $y = f(x)$ over an interval $x \in [a, b]$ with step size $h$:
$$n = \left\lfloor \frac{b - a}{h} \right\rfloor + 1$$

```cpp
for (double x = a; x <= b + h / 2.0; x += h) {
    double y = compute_function(x);
    // Print x and y table row
}
```

---

## 💻 Code & Files

- **Main Code:** [`main.cpp`](./main.cpp)
- **For Loop Code:** [`for_cycle.cpp`](./for_cycle.cpp)
- **While Loop Code:** [`while_cycle.cpp`](./while_cycle.cpp)
- **Lab Report:** [UA DOCX](./Звіт_ЛР2_Циклічні_конструкції_та_перебір.docx)

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
