# 📑 Laboratory Work #2: Binary Relations & Properties

> **Course:** Computer & Discrete Mathematics (CDM)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Study binary relations, matrix representation methods, directed graph representations, and algorithms for verifying core relation properties (reflexivity, irreflexivity, symmetry, anti-symmetry, transitivity).

---

## 📐 Theoretical Background

A **binary relation** $R$ from set $A$ to set $B$ is a subset of the Cartesian product $A \times B$, i.e., $R \subseteq A \times B$. If $A = B$, $R$ is called a binary relation on set $A$.

### Domain and Range

- **Domain ($\text{Dom}(R)$):** The set of all first elements in the ordered pairs: $\text{Dom}(R) = \{ x \in A \mid \exists y \in B, (x, y) \in R \}$.
- **Range ($\text{Ran}(R)$):** The set of all second elements in the ordered pairs: $\text{Ran}(R) = \{ y \in B \mid \exists x \in A, (x, y) \in R \}$.

### Representation Methods

1. **Set of Ordered Pairs:** Explicitly listing pairs $R = \{ (a_1, b_1), (a_2, b_2), \dots \}$.
2. **Boolean Relation Matrix ($M_R$):** An $n \times m$ matrix where elements are defined as:
   $$M_R[i, j] = \begin{cases} 1, & \text{if } (a_i, b_j) \in R \\ 0, & \text{otherwise} \end{cases}$$
3. **Directed Graph (Digraph):** Vertices represent elements of set $A$, and a directed arc exists from $x$ to $y$ if and only if $(x, y) \in R$.

### Core Mathematical Properties of Relations

For a binary relation $R$ on a set $A$:

1. **Reflexivity:** Every element is related to itself:
   $$\forall x \in A, (x, x) \in R \iff \text{diag}(M_R) = (1, 1, \dots, 1)$$
2. **Irreflexivity:** No element is related to itself:
   $$\forall x \in A, (x, x) \notin R \iff \text{diag}(M_R) = (0, 0, \dots, 0)$$
3. **Symmetry:** If $x$ is related to $y$, then $y$ is related to $x$:
   $$\forall x, y \in A, (x, y) \in R \implies (y, x) \in R \iff M_R = M_R^T$$
4. **Anti-symmetry:** If $x$ is related to $y$ and $y$ is related to $x$, then $x$ must equal $y$:
   $$\forall x, y \in A, ((x, y) \in R \land (y, x) \in R) \implies x = y$$
5. **Transitivity:** If $x$ is related to $y$ and $y$ is related to $z$, then $x$ is related to $z$:
   $$\forall x, y, z \in A, ((x, y) \in R \land (y, z) \in R) \implies (x, z) \in R$$

### Equivalence and Order Relations

- **Equivalence Relation:** A relation that is reflexive, symmetric, and transitive. It partitions set $A$ into disjoint equivalence classes.
- **Partial Order Relation:** A relation that is reflexive, anti-symmetric, and transitive.

---

## 💻 Implementation & Code Structure

The implementation [`main.py`](./main.py) uses **NetworkX** and **Matplotlib** to construct matrix representations and render directed graphs of relations.

- `create_relation_matrix(nodes, pairs)`: Converts edge pairs into a boolean adjacency matrix.
- `draw_graph(nodes, pairs)`: Plots directed graph visualization of node interactions.

---

## 🚀 How to Run

1. Install prerequisites:

   ```bash
   pip install networkx matplotlib
   ```

2. Run the application:

   ```bash
   python main.py
   ```

---

## 📄 Guidelines & Documents

- [Lab Guidelines (PDF)](./ЛР2_Відношення_Основні_поняття_та_властивості.pdf)
