import tkinter as tk
from tkinter import scrolledtext, messagebox

def hanoi_logic(n, start_rod, end_rod, extra_rod, move_list):
    """Рекурсивна функція для розв'язання задачі."""
    if n == 1:
        move_list.append(f"Диск 1: Стержень {start_rod} -> {end_rod}")
        return

    hanoi_logic(n - 1, start_rod, extra_rod, end_rod, move_list)
    
    move_list.append(f"Диск {n}: Стержень {start_rod} -> {end_rod}")
    
    hanoi_logic(n - 1, extra_rod, end_rod, start_rod, move_list)

def start_calculation():
    """Функція для запуску розрахунку при натисканні кнопки."""
    try:
        n_disks = int(entry_disks.get())
        
        if n_disks < 1:
            messagebox.showwarning("Помилка", "Кількість дисків має бути більше 0")
            return
        if n_disks > 10:
            messagebox.showwarning("Попередження", "Забагато дисків. Максимум 10.")
            return

        moves = []
        hanoi_logic(n_disks, 1, 3, 2, moves)
        
        output_area.delete("1.0", tk.END)
        output_area.insert(tk.END, f"--- Ханойські башти для {n_disks} дисків ---\n")
        output_area.insert(tk.END, f"Мінімальна кількість кроків: {2**n_disks - 1}\n\n")
        
        for move in moves:
            output_area.insert(tk.END, move + "\n")
            
    except ValueError:
        messagebox.showerror("Помилка", "Будь ласка, введіть ціле число")

app = tk.Tk()
app.title("Лабораторна робота №3: Ханойські башти")
app.geometry("450x550")

frame_input = tk.Frame(app)
frame_input.pack(pady=20)

tk.Label(frame_input, text="Кількість дисків N:").grid(row=0, column=0, padx=10)
entry_disks = tk.Entry(frame_input, width=10)
entry_disks.grid(row=0, column=1)
entry_disks.insert(0, "3")

btn_run = tk.Button(app, text="РОЗРАХВАТИ", command=start_calculation, bg="#f0f0f0", font=("Arial", 9, "bold"))
btn_run.pack(pady=5)

output_area = scrolledtext.ScrolledText(app, width=50, height=20, font=("Consolas", 10))
output_area.pack(pady=10, padx=10)

app.mainloop()