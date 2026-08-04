# 📑 Laboratory Work #7: Shortest Path Algorithms in Graphs

> **Course:** Computer & Discrete Mathematics (CDM)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Study shortest path algorithms on weighted graphs, master Dijkstra's algorithm, implement distance relaxation logic, and reconstruct minimum-cost paths between vertices.

---

## 📐 Theoretical Background

The **Shortest Path Problem** seeks to find a path between two vertices $(u, v)$ in a weighted directed or undirected graph such that the sum of edge weights along the path is minimized:
$$w(P) = \sum_{e \in P} w(e) \to \min$$

### Dijkstra's Algorithm

**Dijkstra's Algorithm** finds the shortest paths from a single source vertex $s$ to all other vertices in a weighted graph with **non-negative edge weights** ($w(e) \ge 0$).

#### Data Structures

- `dist[v]`: Array storing the current shortest distance from source $s$ to vertex $v$.
- `prev[v]`: Array storing the predecessor of vertex $v$ on the shortest path.
- `visited[v]`: Boolean array marking processed vertices.

#### Algorithm Steps

1. **Initialization:**
   $$dist[s] = 0, \quad dist[v] = \infty \quad \forall v \neq s, \quad visited[v] = \text{False} \quad \forall v \in V$$
2. **Vertex Selection:** Find an unvisited vertex $u \in V$ with the minimum tentative distance $dist[u]$.
3. **Mark Visited:** Set $visited[u] = \text{True}$. If $dist[u] = \infty$, remaining unvisited vertices are unreachable; terminate loop.
4. **Edge Relaxation:** For every outgoing neighbor $v$ of $u$:
   $$\text{If } dist[u] + w(u, v) < dist[v] \implies \begin{cases} dist[v] = dist[u] + w(u, v) \\ prev[v] = u \end{cases}$$
5. **Repeat:** Repeat steps 2–4 until all vertices are visited or destination is reached.

### Path Reconstruction

Starting from destination vertex $t$, trace backward through the predecessor array `prev[t]`:
$$t \leftarrow prev[t] \leftarrow prev[prev[t]] \dots \leftarrow s$$
Reversing this sequence yields the optimal shortest path.

### Complexity Analysis

- **Standard Implementation (Array/Matrix):** $\mathcal{O}(|V|^2)$ time complexity — optimal for dense graphs ($|E| \approx |V|^2$).
- **Priority Queue Implementation (Binary Heap):** $\mathcal{O}((|V| + |E|) \log |V|)$ time complexity — optimal for sparse graphs ($|E| \ll |V|^2$).

---

## 💻 Implementation & Code Structure

The application [`main.py`](./main.py) uses **Tkinter**, **NetworkX**, and **Matplotlib** to compute and display step-by-step path solving.

- `dijkstra(graph, start_node)`: Computes `dist` and `prev` arrays.
- `reconstruct_path(prev, start_node, end_node)`: Traces shortest path.
- `draw_graph_with_highlighted_path()`: Renders visual graph with highlighted shortest route.

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

- [Lab Guidelines (PDF)](./ЛР7_Пошук_найкоротших_шляхів_у_графах.pdf)
