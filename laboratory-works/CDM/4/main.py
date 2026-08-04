from sympy import symbols, SOPform
import matplotlib.pyplot as plt

plt.rcParams['toolbar'] = 'None'

def draw_kmap(minterms):
    labels = ['00', '01', '11', '10']
    
    # 4-variable Karnaugh map cell indices mapped using Gray code sequence (00, 01, 11, 10)
    grid_indices = [
        [0, 1, 3, 2],
        [4, 5, 7, 6],
        [12, 13, 15, 14],
        [8, 9, 11, 10]
    ]

    fig, ax = plt.subplots(figsize=(6, 6))
    ax.set_axis_off()
    plt.title("Карта Карно (4 змінні)", pad=20, fontsize=14, fontweight='bold')

    table_data = []
    for row in grid_indices:
        current_row = []
        for index in row:
            val = "1" if index in minterms else "0"
            current_row.append(val)
        table_data.append(current_row)

    table = plt.table(cellText=table_data,
                      rowLabels=labels,
                      colLabels=labels,
                      cellLoc='center',
                      loc='center')

    table.scale(1, 4)
    for (row, col), cell in table.get_celld().items():
        if row == 0 or col == -1:
            cell.set_facecolor("#f2f2f2")
        elif table_data[row-1][col] == "1":
            cell.set_facecolor("#d1e7dd") 
        else:
            cell.set_facecolor("#f8d7da") 

    plt.text(-0.1, 0.5, "x1 x2", va='center', ha='center', fontsize=12, fontweight='bold')
    plt.text(0.5, 0.9, "x3 x4", va='center', ha='center', fontsize=12, fontweight='bold')

    plt.show()

def run_minimization():
    x1, x2, x3, x4 = symbols('x1 x2 x3 x4')
    variables = [x1, x2, x3, x4]

    print("Введіть індекси одиниць через пробіл (0-15):")
    raw_input = input("> ")
    try:
        minterms = [int(i) for i in raw_input.split()]
    except ValueError:
        print("Помилка: введіть лише цілі числа.")
        return

    print("\n--- ТАБЛИЦЯ ІСТИННОСТІ ---")
    print("x1 x2 x3 x4 | f")
    print("-" * 17)
    for i in range(16):
        binary = format(i, '04b')
        val = 1 if i in minterms else 0
        print(f" {'  '.join(binary)}  | {val}")

    minimized_expression = SOPform(variables, minterms)

    print("\n--- РЕЗУЛЬТАТ МІНІМІЗАЦІЇ ---")
    print(f"Спрощений вираз: {minimized_expression}")

    print("\nГенерація карти Карно...")
    draw_kmap(minterms)

if __name__ == "__main__":
    run_minimization()