import math
import networkx as nx
import matplotlib.pyplot as plt


class GraphPathFinder:
    """
    Клас для інкапсуляції логіки роботи з орієнтованим графом.
    """

    def __init__(self, adjacency_matrix):
        """
        Ініціалізація графа на основі матриці суміжності.
        Відсутність дуги позначається як 0 або math.inf.
        """
        self.matrix = adjacency_matrix
        self.n = len(adjacency_matrix)

    def topological_sort(self):
        """
        Алгоритм топологічного сортування вершин.
        Визначає ациклічність та повертає правильну нумерацію.
        """
        in_degree = [0] * self.n
        for u in range(self.n):
            for v in range(self.n):
                if self.matrix[u][v] != 0 and self.matrix[u][v] != math.inf:
                    in_degree[v] += 1

        queue = [i for i in range(self.n) if in_degree[i] == 0]
        topo_order = []

        while queue:
            u = queue.pop(0)
            topo_order.append(u)
            for v in range(self.n):
                if self.matrix[u][v] != 0 and self.matrix[u][v] != math.inf:
                    in_degree[v] -= 1
                    if in_degree[v] == 0:
                        queue.append(v)

        if len(topo_order) != self.n:
            raise ValueError(
                "В графі є цикл. Метод динамічного програмування не застосовується."
            )

        return topo_order

    def find_shortest_path_dp(self, start_idx, end_idx):
        """
        Метод динамічного програмування для знаходження найкоротшого шляху.
        """
        topo_order = self.topological_sort()

        distances = {i: math.inf for i in range(self.n)}
        distances[start_idx] = 0

        for j in topo_order:
            if j == start_idx:
                continue

            min_dist = math.inf
            for i in range(self.n):
                weight = self.matrix[i][j]
                if weight != 0 and weight != math.inf:
                    if distances[i] + weight < min_dist:
                        min_dist = distances[i] + weight
            distances[j] = min_dist

        if distances[end_idx] == math.inf:
            return math.inf, []

        path = [end_idx]
        curr = end_idx

        while curr != start_idx:
            for i in range(self.n):
                weight = self.matrix[i][curr]
                if weight != 0 and weight != math.inf:
                    if math.isclose(distances[curr], distances[i] + weight):
                        curr = i
                        path.insert(0, curr)
                        break

        return distances[end_idx], path

    def visualize_graph(self, shortest_path=None):
        """
        Створює візуальне представлення графа та підсвічує знайдений маршрут.
        """
        G = nx.DiGraph()

        for i in range(self.n):
            for j in range(self.n):
                weight = self.matrix[i][j]
                if weight != 0 and weight != math.inf:
                    G.add_edge(i, j, weight=weight)

        pos = nx.spring_layout(G, seed=42)

        plt.figure(figsize=(8, 6))

        nx.draw_networkx_nodes(G, pos, node_color="lightblue", node_size=800)
        nx.draw_networkx_labels(
            G,
            pos,
            labels={i: str(i + 1) for i in G.nodes()},
            font_size=12,
            font_weight="bold",
        )
        nx.draw_networkx_edges(G, pos, edge_color="gray", arrows=True, arrowsize=15)

        edge_labels = nx.get_edge_attributes(G, "weight")
        edge_labels = {
            k: int(v) if v.is_integer() else v for k, v in edge_labels.items()
        }
        nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_size=10)

        if shortest_path and len(shortest_path) > 1:
            path_edges = [
                (shortest_path[k], shortest_path[k + 1])
                for k in range(len(shortest_path) - 1)
            ]
            nx.draw_networkx_edges(
                G,
                pos,
                edgelist=path_edges,
                edge_color="red",
                width=2.5,
                arrows=True,
                arrowsize=20,
            )

        plt.title("Візуалізація графа (найкоротший шлях виділено червоним)")
        plt.axis("off")
        plt.tight_layout()
        plt.show()


def main():
    print(
        "Лабораторна робота №7: Пошук найкоротших шляхів (Метод динамічного програмування) [cite: 1, 3]"
    )
    try:
        n = int(input("Введіть кількість вершин у графі: "))
        print("\nВводьте матрицю суміжності пострічково (елементи через пробіл).")
        print("Використовуйте '0' для позначення відсутності дуги між вершинами.")

        matrix = []
        for i in range(n):
            row = list(map(float, input(f"Рядок {i + 1}: ").strip().split()))
            if len(row) != n:
                raise ValueError(
                    f"Кількість елементів у рядку повинна дорівнювати {n}."
                )
            matrix.append(row)

        start_vertex = (
            int(input(f"\nВведіть індекс початкової вершини (від 1 до {n}): ")) - 1
        )
        end_vertex = int(input(f"Введіть індекс кінцевої вершини (від 1 до {n}): ")) - 1

        if not (0 <= start_vertex < n) or not (0 <= end_vertex < n):
            raise ValueError("Індекси вершин виходять за межі допустимого діапазону.")

        finder = GraphPathFinder(matrix)
        distance, path = finder.find_shortest_path_dp(start_vertex, end_vertex)

        if distance == math.inf:
            print(
                f"\nШляху від вершини {start_vertex + 1} до {end_vertex + 1} не існує."
            )
            finder.visualize_graph()
        else:
            human_readable_path = [str(v + 1) for v in path]
            print(f"\nМінімальна довжина шляху (L): {distance}")
            print(f"Найкоротший шлях (μ): {' -> '.join(human_readable_path)}")
            finder.visualize_graph(path)

    except ValueError as e:
        print(f"\nПомилка: {e}")


if __name__ == "__main__":
    main()

# Лінійний шлях
# Простий послідовний граф: 1 -> 2 -> 3.
# Вершин: 3
# Матриця:
# 0 5 0
# 0 0 10
# 0 0 0
# Старт: 1 | Фініш: 3
# Очікувано: L = 15, шлях: 1 -> 2 -> 3

# Розгалуження (обхідний шлях швидший)
# Прямий шлях дорожчий за обхідний через іншу вершину.
# Вершин: 4
# Матриця:
# 0 10 2 0
# 0 0 0 5
# 0 0 0 2
# 0 0 0 0
# Старт: 1 | Фініш: 4
# Очікувано: L = 4, шлях: 1 -> 3 -> 4

# Щільний ациклічний граф
# Існують всі можливі прямі ребра від меншого індексу до більшого.
# Вершин: 4
# Матриця:
# 0 1 5 10
# 0 0 1 5
# 0 0 0 1
# 0 0 0 0
# Старт: 1 | Фініш: 4
# Очікувано: L = 3, шлях: 1 -> 2 -> 3 -> 4

# Граф із циклом (Exception)
# Перевірка спрацьовування захисту від зациклення.
# Вершин: 3
# Матриця:
# 0 5 0
# 0 0 5
# 5 0 0
# Старт: 1 | Фініш: 3
# Очікувано: ValueError (алгоритм топологічного сортування має виявити цикл).

# Зворотна нумерація
# Перевірка топологічного сортування на графі, де ребра йдуть від більшого індексу до меншого.
# Вершин: 3
# Матриця:
# 0 0 0
# 5 0 0
# 0 10 0
# Старт: 3 | Фініш: 1
# Очікувано: L = 15, шлях: 3 -> 2 -> 1

# Острів (Недосяжна вершина)
# Кінцева вершина повністю ізольована.
# Вершин: 3
# Матриця:
# 0 5 0
# 0 0 0
# 0 0 0
# Старт: 1 | Фініш: 3
# Очікувано: Шляху не існує (нескінченність).

# Вироджений граф (Старт = Фініш)
# Запит на відстань від вершини до неї самої.
# Вершин: 1
# Матриця:
# 0
# Старт: 1 | Фініш: 1
# Очікувано: L = 0, шлях: 1

# Нульова матриця
# Граф без жодного ребра.
# Вершин: 4
# Матриця:
# 0 0 0 0
# 0 0 0 0
# 0 0 0 0
# 0 0 0 0
# Старт: 1 | Фініш: 4
# Очікувано: Шляху не існує.

# Декілька однакових за вагою шляхів
# Шляхи 1 -> 2 -> 4 та 1 -> 3 -> 4 мають ідентичну вартість.
# Вершин: 4
# Матриця:
# 0 5 5 0
# 0 0 0 5
# 0 0 0 5
# 0 0 0 0
# Старт: 1 | Фініш: 4
# Очікувано: L = 10. (Шлях 1 -> 2 -> 4 або 1 -> 3 -> 4).

# Довгий зигзаг кращий за пряму
# Пряме ребро до цілі має величезну вагу порівняно з каскадом дрібних переходів.
# Вершин: 4
# Матриця:
# 0 100 5 0
# 0 0 0 0
# 0 1 0 1
# 0 0 0 0
# Старт: 1 | Фініш: 2
# Очікувано: L = 6, шлях: 1 -> 3 -> 2.
