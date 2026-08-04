# 📑 Laboratory Work #6: Matrix Representations of Graphs

> **Course:** Computer & Discrete Mathematics (CDM)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Study matrix methods for representing graphs, construct adjacency, incidence, distance, and reachability matrices, and evaluate graph path properties using matrix power operations.

---

## 📐 Theoretical Background

Matrix representations provide structured data structures for graph algorithms and algebraic analysis of topological graph structures.

### 1. Adjacency Matrix ($A$)

For a graph $G = (V, E)$ with $|V| = n$, the **Adjacency Matrix** $A$ is an $n \times n$ square boolean matrix:
$$A_{i,j} = \begin{cases} 1, & \text{if } (v_i, v_j) \in E \\ 0, & \text{otherwise} \end{cases}$$

- **Undirected Graphs:** $A$ is symmetric ($A = A^T$). Sum of row $i$ equals $\deg(v_i)$.
- **Directed Graphs:** Sum of row $i$ equals out-degree $\deg^+(v_i)$; sum of column $j$ equals in-degree $\deg^-(v_j)$.

### 2. Incidence Matrix ($B$)

For a graph with $n$ vertices and $m$ edges ($|E| = m$), the **Incidence Matrix** $B$ is an $n \times m$ matrix:

- **Undirected Graph:**
  $$B_{i,j} = \begin{cases} 1, & \text{if vertex } v_i \text{ is incident to edge } e_j \\ 0, & \text{otherwise} \end{cases}$$
- **Directed Graph:**
  $$B_{i,j} = \begin{cases} -1, & \text{if } v_i \text{ is the source of arc } e_j \\ +1, & \text{if } v_i \text{ is the target of arc } e_j \\ 0, & \text{otherwise} \end{cases}$$

### 3. Reachability ($R$) and Distance ($D$) Matrices

- **Reachability Matrix ($R$):**
  $$R_{i,j} = \begin{cases} 1, & \text{if there exists a path of any length from } v_i \text{ to } v_j \\ 0, & \text{otherwise} \end{cases}$$
- **Distance Matrix ($D$):**
  $$D_{i,j} = \text{length of the shortest path from } v_i \text{ to } v_j \quad (D_{i,j} = \infty \text{ if unreachable})$$

### Paths of Length $k$ via Matrix Multiplication

A key algebraic property of adjacency matrices:
$$(A^k)_{i,j} = \text{number of distinct paths of exact length } k \text{ from vertex } v_i \text{ to vertex } v_j$$
The reachability matrix can be computed via Boolean matrix addition:
$$R = I \lor A \lor A^2 \lor A^3 \lor \dots \lor A^{n-1}$$

---

## 💻 Implementation & Code Structure

The application [`main.py`](./main.py) uses **Tkinter**, **NetworkX**, and **Matplotlib** to build and display matrix representations.

- `build_adjacency_matrix()`: Generates $A_{n \times n}$.
- `build_incidence_matrix()`: Generates $B_{n \times m}$.
- `compute_paths_of_length_k(A, k)`: Computes matrix powers $A^k$ using `numpy.linalg.matrix_power`.

---

## 🚀 How to Run

1. Install prerequisites:

   ```bash
   pip install numpy networkx matplotlib
   ```

2. Run the application:

   ```bash
   python main.py
   ```

---

## 📄 Guidelines & Documents

- [Lab Guidelines (PDF)](./ЛР6_Матричні_способи_представлення_графів.pdf)
