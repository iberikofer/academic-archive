# 📑 Laboratory Work #4: Minimization of Boolean Logic Functions

> **Course:** Computer & Discrete Mathematics (CDM)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Master techniques for analyzing Boolean logic functions, constructing truth tables, deriving canonical normal forms (PDNF/PCNF), and performing function minimization using Karnaugh maps.

---

## 📐 Theoretical Background

A **Boolean function** of $n$ variables is a mapping $f: \{0, 1\}^n \to \{0, 1\}$. Truth tables enumerate all $2^n$ possible input combinations and corresponding output values.

### Canonical Forms

1. **Principal Disjunctive Normal Form (PDNF / DNF):** A logical OR ($\lor$) of minterms (conjunctions where each variable appears once in direct or negated form) corresponding to rows where $f = 1$:
   $$f(x_1, \dots, x_n) = \bigvee_{f(a_1,\dots,a_n)=1} m_{(a_1,\dots,a_n)}$$
2. **Principal Conjunctive Normal Form (PCNF / CNF):** A logical AND ($\land$) of maxterms (disjunctions) corresponding to rows where $f = 0$:
   $$f(x_1, \dots, x_n) = \bigwedge_{f(a_1,\dots,a_n)=0} M_{(a_1,\dots,a_n)}$$

### Karnaugh Maps (K-Maps) Minimization

A **Karnaugh map** is a 2D grid representation of a truth table structured using Gray code order (where adjacent cells differ by exactly one binary digit).

#### Minimization Rules

1. Arrange 1s in rectangular groups (subcubes) of sizes equal to powers of two ($2^k = 1, 2, 4, 8, 16$).
2. Wrap-around rule applies: opposite top-bottom and left-right edges are adjacent.
3. Form maximal groupings to yield minimal product terms (**Prime Implicants**).
4. Identify **Essential Prime Implicants** (groups containing at least one 1 not covered by any other group).

### Logic Gate Equivalence

Boolean minimization reduces circuit complexity and gate count:

- **AND ($\land$):** $f = x_1 \cdot x_2$
- **OR ($\lor$):** $f = x_1 + x_2$
- **NOT ($\neg$):** $f = \overline{x_1}$
- **XOR ($\oplus$):** $f = x_1 \oplus x_2 = x_1 \overline{x_2} \lor \overline{x_1} x_2$

---

## 💻 Implementation & Code Structure

The application [`main.py`](./main.py) uses **Tkinter** to construct interactive truth tables and generate minimized Boolean expressions.

- `build_truth_table(func_expr)`: Computes output for all $2^n$ combinations.
- `kmap_minimize()`: Groups minterms and displays the simplified boolean formula.

---

## 🚀 How to Run

1. Run the application:

   ```bash
   python main.py
   ```

---

## 📄 Guidelines & Documents

- [Lab Guidelines (PDF)](./ЛР4_Мінімізація_логічних_функцій.pdf)
