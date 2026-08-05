import numpy as np
import networkx as nx
import matplotlib.pyplot as plt
from typing import Tuple

class GraphConverter:
    """Клас для конвертації між матрицями суміжності та інцидентності."""
    
    @staticmethod
    def adj_to_inc(adj_matrix: np.ndarray, graph_type: str) -> np.ndarray:
        n = adj_matrix.shape[0]
        edges = []
        
        if graph_type in ["directed", "mixed"]:
            for i in range(n):
                for j in range(n):
                    if adj_matrix[i, j] > 0:
                        edges.append((i, j))
            
            inc_matrix = np.zeros((n, len(edges)), dtype=int)
            for idx, (u, v) in enumerate(edges):
                inc_matrix[u, idx] = 1
                if u != v:
                    inc_matrix[v, idx] = -1
                    
        else:
            for i in range(n):
                for j in range(i, n):
                    if adj_matrix[i, j] > 0:
                        edges.append((i, j))
                        
            inc_matrix = np.zeros((n, len(edges)), dtype=int)
            for idx, (u, v) in enumerate(edges):
                inc_matrix[u, idx] = 1
                inc_matrix[v, idx] = 1
                
        return inc_matrix

    @staticmethod
    def inc_to_adj(inc_matrix: np.ndarray, graph_type: str) -> np.ndarray:
        n, m = inc_matrix.shape
        adj_matrix = np.zeros((n, n), dtype=int)
        
        for j in range(m):
            col = inc_matrix[:, j]
            
            if graph_type in ["directed", "mixed"]:
                starts = np.where(col == 1)[0]
                ends = np.where(col == -1)[0]
                
                if len(starts) == 1 and len(ends) == 1:
                    adj_matrix[starts[0], ends[0]] = 1
                elif len(starts) == 1 and len(ends) == 0:
                    adj_matrix[starts[0], starts[0]] = 1 
            else:
                nodes = np.where(col == 1)[0]
                if len(nodes) == 2:
                    adj_matrix[nodes[0], nodes[1]] = 1
                    adj_matrix[nodes[1], nodes[0]] = 1
                elif len(nodes) == 1:
                    adj_matrix[nodes[0], nodes[0]] = 1
                    
        return adj_matrix


class GraphVisualizer:
    """Клас для відображення графів."""
    
    @staticmethod
    def show_graph(adj_matrix: np.ndarray, graph_type: str):
        plt.figure(figsize=(8, 6))
        
        if graph_type in ["directed", "mixed"]:
            G = nx.DiGraph(adj_matrix)
        else:
            G = nx.Graph(adj_matrix)
            
        pos = nx.spring_layout(G, seed=42)
        nx.draw_networkx(G, pos, with_labels=True, node_color='#76b900', 
                        node_size=800, font_weight='bold', font_size=12, 
                        font_color='white', arrows=(graph_type != "undirected"), arrowsize=20)
                
        plt.title(f"Візуалізація графа ({graph_type})")
        plt.axis('off')
        plt.show()


class ConsoleApp:
    """Головний клас додатку для консольної взаємодії."""
    
    @staticmethod
    def read_matrix_from_console(rows: int) -> np.ndarray:
        print(f"Введіть матрицю по рядках (елементи через пробіл):")
        matrix = []
        for i in range(rows):
            while True:
                try:
                    row = list(map(int, input(f"Рядок {i+1}: ").strip().split()))
                    matrix.append(row)
                    break
                except ValueError:
                    print("Помилка вводу. Введіть цілі числа через пробіл.")
        return np.array(matrix)

    def run(self):
        print("=== Лабораторна робота №6: Матричні представлення графів ===")
        
        print("\nОберіть тип графа:")
        print("1. Орієнтований")
        print("2. Неорієнтований")
        print("3. Змішаний")
        g_choice = input("Ваш вибір (1/2/3): ").strip()
        
        type_map = {"1": "directed", "2": "undirected", "3": "mixed"}
        graph_type = type_map.get(g_choice, "directed")
        
        print("\nОберіть напрямок конвертації:")
        print("1. Матриця суміжності -> Матриця інцидентності")
        print("2. Матриця інцидентності -> Матриця суміжності")
        c_choice = input("Ваш вибір (1/2): ").strip()
        
        if c_choice == "1":
            n = int(input("\nВведіть кількість вершин графа (n): ").strip())
            adj_matrix = self.read_matrix_from_console(n)
            inc_matrix = GraphConverter.adj_to_inc(adj_matrix, graph_type)
            
            print("\n--- Результат: Матриця інцидентності ---")
            print(inc_matrix)
            GraphVisualizer.show_graph(adj_matrix, graph_type)
            
        elif c_choice == "2":
            n = int(input("\nВведіть кількість вершин (n): ").strip())
            m = int(input("Введіть кількість ребер/дуг (m): ").strip())
            inc_matrix = self.read_matrix_from_console(n)
            adj_matrix = GraphConverter.inc_to_adj(inc_matrix, graph_type)
            
            print("\n--- Результат: Матриця суміжності ---")
            print(adj_matrix)
            GraphVisualizer.show_graph(adj_matrix, graph_type)
            
        else:
            print("Невірний вибір. Завершення програми.")

if __name__ == "__main__":
    app = ConsoleApp()
    app.run()

# Неорієнтований граф
# 2 2 4 3
# 1 0 0
# 1 1 0
# 0 1 1
# 0 0 1

# Орієнтований граф
# 1 1 3
# 0 1 0
# 0 0 1
# 1 0 0

# Орієнтований граф з петлею 
# 1 1 3
# 1 1 0
# 0 0 1
# 0 0 0

# ромб
# 0 1 1 0
# 0 0 0 1
# 0 0 0 1
# 0 0 0 0

# Зірка
# 2 1 5
# 0 1 1 1 1
# 1 0 0 0 0
# 1 0 0 0 0
# 1 0 0 0 0
# 1 0 0 0 0