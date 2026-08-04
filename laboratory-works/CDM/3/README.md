# 📑 Laboratory Work #3: Recursive Algorithms & Tower of Hanoi

> **Course:** Computer & Discrete Mathematics (CDM)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Study algorithmic properties, recursion mechanisms, call stack behavior, recurrence relation solving, and the classical Tower of Hanoi mathematical puzzle.

---

## 📐 Theoretical Background

An **algorithm** is a finite sequence of well-defined instructions for solving a specific problem. Key properties of algorithms include:

1. **Finiteness (Determinacy):** Terminating after a finite number of steps.
2. **Definiteness (Unambiguity):** Each step is precisely defined.
3. **Input / Output:** Accepts zero or more inputs and produces one or more outputs.
4. **Effectiveness:** Each operation is basic enough to be carried out exactly.

### Recursion and Call Stack

**Recursion** occurs when a function calls itself directly or indirectly to solve a smaller instance of the same problem. A recursive algorithm must contain:

- **Base Case (Termination Condition):** The simplest scenario solved directly without recursive calls.
- **Recursive Step:** Reducing the problem size toward the base case.

Recursive execution utilizes the program execution stack (call stack) to preserve local variables and return addresses for active function invocations.

### Tower of Hanoi Mathematical Model

The **Tower of Hanoi** puzzle consists of 3 rods ($A, B, C$) and $n$ disks of different sizes.

- **Rules:**
  1. Only one disk can be moved at a time.
  2. A larger disk cannot be placed on top of a smaller disk.
  3. Disks must be moved from the source rod to the destination rod using an auxiliary rod.

#### Recurrence Relation & Solution

Let $T(n)$ be the minimum number of moves required to solve the puzzle for $n$ disks:
$$T(n) = 2T(n-1) + 1, \quad T(1) = 1$$

Expanding the recurrence:
$$T(n) = 2(2T(n-2) + 1) + 1 = 2^2 T(n-2) + 2 + 1$$
$$T(n) = 2^{n-1} T(1) + 2^{n-2} + \dots + 2^1 + 2^0 = \sum_{k=0}^{n-1} 2^k = 2^n - 1$$

Thus, the minimum number of moves for $n$ disks is $T(n) = 2^n - 1$, yielding an exponential time complexity of $\mathcal{O}(2^n)$ and stack space complexity of $\mathcal{O}(n)$.

---

## 💻 Implementation & Code Structure

The application [`main.py`](./main.py) provides an interactive GUI built with **Tkinter** for animating disk movements and logging recursion call stacks.

- `hanoi(n, source, target, auxiliary)`: Recursive function logging moves and updating visual state.
- `step_counter`: Tracks total executed moves to verify $T(n) = 2^n - 1$.

---

## 🚀 How to Run

1. Run the application:

   ```bash
   python main.py
   ```

---

## 📄 Guidelines & Documents

- [Lab Guidelines (PDF)](./ЛР3_Алгоритми_Рекурсія.pdf)
