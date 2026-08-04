import tkinter as tk
from tkinter import ttk
import networkx as nx
import math
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

class Lab5GraphApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Лаба5")
        self.root.geometry("1000x650")
        self.root.configure(bg="#f0f0f0")

        self.variants = {
            "1a": [
                [0, 0, 0, 1, 1, 0, 0],
                [1, 0, 1, 1, 0, 1, 1],
                [1, 1, 0, 1, 1, 0, 1],
                [0, 0, 0, 0, 1, 1, 0],
                [1, 0, 0, 1, 0, 1, 0],
                [0, 1, 1, 1, 0, 0, 0],
                [1, 1, 0, 0, 1, 1, 0]
            ],
            "1b": [
                [0, 0, 0, 0, 1, 1, 0],
                [1, 0, 1, 0, 1, 0, 1],
                [1, 1, 0, 1, 1, 0, 0],
                [0, 0, 1, 0, 1, 1, 0],
                [1, 0, 0, 1, 0, 1, 0],
                [0, 0, 1, 0, 1, 0, 0],
                [1, 1, 0, 1, 1, 0, 0]
            ],
            "1c": [
                [0, 0, 0, 1, 0, 1, 0],
                [1, 0, 1, 0, 1, 0, 1],
                [0, 1, 0, 1, 1, 1, 0],
                [1, 0, 0, 0, 1, 0, 0],
                [1, 0, 1, 1, 0, 1, 0],
                [0, 0, 0, 1, 1, 0, 0],
                [1, 0, 1, 0, 1, 1, 0]
            ],
            "2a": [
                [math.inf, math.inf, 5, 4, 2, 2, 9],
                [math.inf, math.inf, 1, 1, math.inf, 1, 1],
                [2, math.inf, math.inf, 1, 1, math.inf, 3],
                [math.inf, 2, 1, math.inf, 1, math.inf, math.inf],
                [math.inf, math.inf, 2, 2, math.inf, 1, 6],
                [1, 5, math.inf, 1, 1, math.inf, math.inf],
                [2, math.inf, 1, math.inf, 1, 2, math.inf]
            ],
            "2b": [
                [math.inf, 4, math.inf, math.inf, 5, 4, math.inf],
                [9, math.inf, 2, 1, math.inf, math.inf, math.inf],
                [4, 4, math.inf, math.inf, math.inf, math.inf, 3],
                [math.inf, 3, 1, math.inf, 1, math.inf, math.inf],
                [math.inf, math.inf, 2, math.inf, math.inf, 4, 5],
                [math.inf, 3, math.inf, 2, 2, math.inf, math.inf],
                [math.inf, math.inf, 2, math.inf, math.inf, 2, math.inf]
            ],
            "2c": [
                [math.inf, math.inf, 9, math.inf, math.inf, 2, 12],
                [1, math.inf, math.inf, math.inf, 1, 2, 4],
                [2, 1, math.inf, math.inf, 1, math.inf, 2],
                [math.inf, 4, 1, math.inf, math.inf, 1, math.inf],
                [1, 2, math.inf, 2, math.inf, math.inf, math.inf],
                [math.inf, math.inf, math.inf, math.inf, 4, math.inf, 5],
                [math.inf, 2, 1, math.inf, 1, 2, math.inf]
            ]
        }

        self.setup_ui()
        self.update_graph()

    def setup_ui(self):
        top_frame = tk.Frame(self.root, bg="#f0f0f0")
        top_frame.pack(fill=tk.X, padx=10, pady=10)

        tk.Label(top_frame, text="Варіант:", bg="#f0f0f0", font=("Arial", 10)).pack(side=tk.LEFT)
        self.variant_var = tk.StringVar(value="1a")
        self.combo = ttk.Combobox(top_frame, textvariable=self.variant_var, values=list(self.variants.keys()), width=5, state="readonly")
        self.combo.pack(side=tk.LEFT, padx=5)
        self.combo.bind("<<ComboboxSelected>>", lambda e: self.update_graph())

        main_frame = tk.Frame(self.root, bg="#f0f0f0")
        main_frame.pack(fill=tk.BOTH, expand=True, padx=10)

        self.fig = Figure(figsize=(6, 5), dpi=100)
        self.fig.patch.set_facecolor('#f0f0f0')
        self.ax = self.fig.add_subplot(111)
        self.ax.set_facecolor('#f0f0f0')
        self.canvas = FigureCanvasTkAgg(self.fig, master=main_frame)
        self.canvas.get_tk_widget().pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        right_frame = tk.Frame(main_frame, bg="#f0f0f0")
        right_frame.pack(side=tk.RIGHT, fill=tk.Y, padx=20)
        
        tk.Label(right_frame, text="матриця:", bg="#f0f0f0", font=("Arial", 10, "italic"), fg="gray").pack(anchor=tk.N, pady=(0, 5))
        self.matrix_text = tk.Text(right_frame, width=32, height=15, font=("Courier New", 12), bg="#f0f0f0", bd=0, highlightthickness=0)
        self.matrix_text.pack(fill=tk.BOTH, expand=True)

        bottom_frame = tk.Frame(self.root, bg="#f0f0f0")
        bottom_frame.pack(fill=tk.X, padx=10, pady=10)

        self.result_label = tk.Label(bottom_frame, text="", bg="#f0f0f0", font=("Arial", 10))
        self.result_label.pack(side=tk.LEFT)

    def update_graph(self):
        var_id = self.variant_var.get()
        matrix = self.variants[var_id]
        is_weighted = var_id.startswith("2")

        matrix_str = ""
        for row in matrix:
            row_items = []
            for val in row:
                if val == math.inf:
                    row_items.append(f"{'∞':>3}")
                else:
                    row_items.append(f"{int(val):>3}")
            matrix_str += "  ".join(row_items) + "\n\n"

        self.matrix_text.config(state=tk.NORMAL)
        self.matrix_text.delete("1.0", tk.END)
        self.matrix_text.insert(tk.END, matrix_str)
        self.matrix_text.config(state=tk.DISABLED)

        G = nx.DiGraph()
        n = len(matrix)

        for i in range(n):
            for j in range(n):
                w = matrix[i][j]
                if w != 0 and w != math.inf:
                    G.add_edge(f"v{i+1}", f"v{j+1}", weight=w)

        try:
            path = nx.bellman_ford_path(G, "v1", "v7", weight="weight")
            length = nx.bellman_ford_path_length(G, "v1", "v7", weight="weight")
            path_str = " -> ".join(path)
            self.result_label.config(text=f"{var_id}: {path_str} (довжина={int(length)})")
        except nx.NetworkXNoPath:
            path = []
            self.result_label.config(text=f"{var_id}: Шлях від v1 до v7 відсутній у цьому графі")

        self.ax.clear()
        
        pos = nx.circular_layout(G)

        nx.draw_networkx_nodes(G, pos, ax=self.ax, node_color="#b2dfdb", node_size=800, edgecolors="gray")
        nx.draw_networkx_labels(G, pos, ax=self.ax, font_size=10, font_color="#333333")

        edges = list(G.edges())
        path_edges = list(zip(path, path[1:])) if path else []
        normal_edges = [e for e in edges if e not in path_edges]

        nx.draw_networkx_edges(G, pos, ax=self.ax, edgelist=normal_edges, arrowsize=15, edge_color="#b0b0b0", node_size=800, connectionstyle="arc3,rad=0.1")
        nx.draw_networkx_edges(G, pos, ax=self.ax, edgelist=path_edges, arrowsize=20, edge_color="#d32f2f", width=2.5, node_size=800, connectionstyle="arc3,rad=0.1")

        if is_weighted:
            edge_labels = nx.get_edge_attributes(G, 'weight')
            edge_labels_int = {k: int(v) for k, v in edge_labels.items()}
            nx.draw_networkx_edge_labels(G, pos, ax=self.ax, edge_labels=edge_labels_int, font_size=9, font_color="blue", label_pos=0.3)

        self.ax.axis("off")
        self.fig.subplots_adjust(left=0, right=1, top=1, bottom=0)
        self.canvas.draw()

if __name__ == "__main__":
    root = tk.Tk()
    app = Lab5GraphApp(root)
    root.mainloop()