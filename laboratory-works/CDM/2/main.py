import matplotlib.pyplot as plt
import networkx as nx
import re
import heapq

plt.rcParams['toolbar'] = 'None'

def create_relation_matrix(nodes_list, pairs_list):
    matrix_size = len(nodes_list)
    matrix = [[0] * matrix_size for _ in range(matrix_size)]
    index_map = {value: i for i, value in enumerate(nodes_list)}
    
    for start_node, end_node in pairs_list:
        if start_node in index_map and end_node in index_map:
            row = index_map[start_node]
            col = index_map[end_node]
            matrix[row][col] = 1
    return matrix

def draw_graph(nodes_list, pairs_list):
    graph_obj = nx.DiGraph()
    graph_obj.add_nodes_from(nodes_list)
    graph_obj.add_edges_from(pairs_list)
    plt.figure(figsize=(7, 5))
    nx.draw(graph_obj, with_labels=True, node_color='lightgreen', arrowsize=20)
    plt.title("Graph Visualization")
    plt.show()

def reverse_array(data_list):
    total_elements = len(data_list)
    for i in range(total_elements // 2):
        data_list[i], data_list[total_elements - 1 - i] = data_list[total_elements - 1 - i], data_list[i]

def transpose_square_matrix(matrix_data):
    side_size = len(matrix_data)
    for i in range(side_size):
        for j in range(i + 1, side_size):
            matrix_data[i][j], matrix_data[j][i] = matrix_data[j][i], matrix_data[i][j]

def search_saddle_points(matrix_data):
    points_list = []
    for row_idx, current_row in enumerate(matrix_data):
        min_in_row = min(current_row)
        for col_idx, cell_value in enumerate(current_row):
            if cell_value == min_in_row:
                current_column = [matrix_data[r][col_idx] for r in range(len(matrix_data))]
                if cell_value == max(current_column):
                    points_list.append((row_idx, col_idx, cell_value))
    return points_list

def generate_m_sequence(limit=100):
    priority_queue = [1]
    m_values = []
    unique_elements = {1}
    
    while len(m_values) < limit:
        smallest = heapq.heappop(priority_queue)
        m_values.append(smallest)
        
        for variant in [2 * smallest + 1, 3 * smallest + 1]:
            if variant not in unique_elements:
                unique_elements.add(variant)
                heapq.heappush(priority_queue, variant)
    return m_values

def main():
    print("Завдання 1")
    input_nodes = input("Елементи: ").split()
    input_pairs_raw = input("Пари (через дефіс): ").split()
    formatted_pairs = [tuple(re.split(r'[-]+', p)) for p in input_pairs_raw if '-' in p]
    
    final_matrix = create_relation_matrix(input_nodes, formatted_pairs)
    print("Матриця:")
    for row in final_matrix:
        print(row)

    print("\nЗавдання 2")
    vector_a = list(range(1, 28))
    reverse_array(vector_a)
    print(f"Реверс A(27) виконано. Початок: {vector_a[:5]}")

    print("\nЗавдання 3")
    matrix_10x10 = [[(i * 10 + j) for j in range(10)] for i in range(10)]
    transpose_square_matrix(matrix_10x10)
    print("Транспонування 10x10 виконано.")

    print("\nЗавдання 4")
    data_matrix = [[3, 8, 7], [4, 7, 6], [2, 6, 5]]
    saddle_results = search_saddle_points(data_matrix)
    if not saddle_results:
        print("Сідлових точок не знайдено.")
    else:
        print(f"Знайдені точки: {saddle_results}")

    print("\nЗавдання 5")
    sequence_m = generate_m_sequence(100)
    print(f"Перші 10 елементів M: {sequence_m[:10]}")

    draw_graph(input_nodes, formatted_pairs)

if __name__ == "__main__":
    main()