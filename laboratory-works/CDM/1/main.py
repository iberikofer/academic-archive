import tkinter as tk
from tkinter import scrolledtext, messagebox
import matplotlib.pyplot as plt
from matplotlib_venn import venn2
import itertools
import re

plt.rcParams['toolbar'] = 'None'

def get_all_subsets(source_set):
    items = list(source_set)
    subsets_list = []
    for size in range(len(items) + 1):
        for combo in itertools.combinations(items, size):
            subsets_list.append(set(combo))
    return subsets_list

def process_data():
    text_a = field_a.get()
    text_b = field_b.get()
    
    set_a = {item for item in re.split(r'[,\s;]+', text_a.strip()) if item}
    set_b = {item for item in re.split(r'[,\s;]+', text_b.strip()) if item}

    if len(set_a) > 10:
        messagebox.showwarning("Обмеження", f"У множині A забагато елементів ({len(set_a)}). \nМаксимально дозволено 10, щоб програма не зависла.")
        return 

    log_area.delete("1.0", tk.END)
    log_area.insert(tk.END, "--- РЕЗУЛЬТАТИ ПЕРЕВІРКИ ---\n")
    
    if set_a == set_b:
        log_area.insert(tk.END, "Множини рівні: A = B\n")
    elif set_a.issubset(set_b):
        log_area.insert(tk.END, "Відношення: A ⊆ B\n")
    elif set_b.issubset(set_a):
        log_area.insert(tk.END, "Відношення: B ⊆ A\n")
    else:
        log_area.insert(tk.END, "Відношення включення не знайдено\n")

    subsets = get_all_subsets(set_a)
    log_area.insert(tk.END, f"\n--- УСІ ПІДМНОЖИНИ A (Кількість: {2**len(set_a)}) ---\n")
    formatted_subsets = [str(s) if s else "∅" for s in subsets]
    log_area.insert(tk.END, ", ".join(formatted_subsets) + "\n")

    plt.close('all')
    fig, axes = plt.subplots(2, 2, figsize=(9, 7))
    fig.canvas.manager.set_window_title('Діаграми Венна')

    operations = [
        ("A ∪ B (Об'єднання)", "union"),
        ("A ∩ B (Перетин)", "intersection"),
        ("A - B (Різниця)", "difference"),
        ("A Δ B (Сим. різниця)", "symmetric")
    ]

    for i, (title, op_type) in enumerate(operations):
        ax = axes.flat[i]
        venn = venn2(subsets=(1, 1, 0.5), set_labels=('A', 'B'), ax=ax)
        ax.set_title(title)
        
        if not venn: continue

        for zone in ['10', '01', '11']:
            patch = venn.get_patch_by_id(zone)
            if patch:
                patch.set_alpha(0.1)
                patch.set_color('gray')

        active_color = "red"
        if op_type == "union":
            for z in ['10', '11', '01']: 
                p = venn.get_patch_by_id(z)
                if p: p.set_alpha(0.7); p.set_color(active_color)
        elif op_type == "intersection":
            p = venn.get_patch_by_id('11')
            if p: p.set_alpha(0.7); p.set_color(active_color)
        elif op_type == "difference":
            p = venn.get_patch_by_id('10')
            if p: p.set_alpha(0.7); p.set_color(active_color)
        elif op_type == "symmetric":
            for z in ['10', '01']:
                p = venn.get_patch_by_id(z)
                if p: p.set_alpha(0.7); p.set_color(active_color)

    plt.tight_layout()
    plt.show()

app = tk.Tk()
app.title("Лабораторна робота №1")
app.geometry("500x500")

frame_top = tk.Frame(app)
frame_top.pack(pady=15)

tk.Label(frame_top, text="Множина A:").grid(row=0, column=0)
field_a = tk.Entry(frame_top, width=40)
field_a.grid(row=0, column=1, padx=5, pady=5)
field_a.insert(0, "1, 2, 3")

tk.Label(frame_top, text="Множина B:").grid(row=1, column=0)
field_b = tk.Entry(frame_top, width=40)
field_b.grid(row=1, column=1, padx=5, pady=5)
field_b.insert(0, "3, 4, 5")

btn_start = tk.Button(app, text="ВИКОНАТИ АНАЛІЗ", command=process_data, 
                      bg="#f0f0f0", font=("Arial", 9, "bold"))
btn_start.pack(pady=5)

log_area = scrolledtext.ScrolledText(app, width=65, height=18, font=("Consolas", 9))
log_area.pack(pady=10, padx=10)

app.mainloop()