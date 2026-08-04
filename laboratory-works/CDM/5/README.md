# 📑 Laboratory Work #5: Fundamentals of Graph Theory

> **Course:** Computer & Discrete Mathematics (CDM)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Learn graph theory fundamentals, vertex degree calculations, graph connectivity analysis, and algorithms for finding routes between target vertices (e.g., $v_1 \to v_7$) in directed graphs.

---

## 📐 Theoretical Background

A **graph** $G = (V, E)$ consists of a non-empty set of vertices (nodes) $V = \{ v_1, v_2, \dots, v_n \}$ and a set of edges (arcs) $E = \{ e_1, e_2, \dots, e_m \}$, where each edge connects a pair of vertices $(v_i, v_j)$.

### Graph Classification

- **Undirected Graph:** Edges are unordered pairs $\{v_i, v_j\}$, representing symmetric bidirectional relationships.
- **Directed Graph (Digraph):** Edges are ordered pairs $(v_i, v_j)$, directed from source vertex $v_i$ to target vertex $v_j$.
- **Weighted Graph:** A weight function $w: E \to \mathbb{R}$ assigns numerical values (distances, costs, capacities) to edges.

### Degree of Vertices & Handshaking Lemma

- In an undirected graph, the **degree** $\deg(v)$ of vertex $v$ is the number of edges incident to $v$.
- **Handshaking Lemma:** The sum of degrees of all vertices equals twice the total number of edges:
  $$\sum_{v \in V} \deg(v) = 2|E|$$
- In a directed graph, the degree split into:
  - **In-degree ($\deg^-(v)$):** Number of incoming arcs to $v$.
  - **Out-degree ($\deg^+(v)$):** Number of outgoing arcs from $v$.
  $$\deg(v) = \deg^-(v) + \deg^+(v)$$
  $$\sum_{v \in V} \deg^-(v) = \sum_{v \in V} \deg^+(v) = |E|$$

### Paths, Cycles, and Connectivity

- **Path:** A sequence of alternating vertices and edges $(v_0, e_1, v_1, e_2, \dots, e_k, v_k)$ connecting $v_0$ to $v_k$.
- **Simple Path:** A path in which all vertices are distinct.
- **Cycle:** A closed path where $v_0 = v_k$.
- **Eulerian Path / Cycle:** A path/cycle that visits every edge in graph $G$ exactly once.
- **Hamiltonian Path / Cycle:** A path/cycle that visits every vertex in graph $G$ exactly once.

---

## 💻 Implementation & Code Structure

The application [`main.py`](./main.py) features an interactive **Tkinter + NetworkX** GUI application (`Lab5GraphApp`).

- Allows selecting pre-configured matrix variants.
- Renders graph layout dynamically with embedded Matplotlib canvas.
- Calculates path reachability from source node $v_1$ to destination $v_7$.

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

- [Lab Guidelines (PDF)](./ЛР5_Основні_поняття_теорії_графів.pdf)
