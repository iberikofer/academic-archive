<a id="-english-version"></a>

# 🇬🇧 Coursework Explanatory Note & Academic Documentation

> 🌐 **Language:** English &nbsp;|&nbsp; **[🇺🇦 Switch to Ukrainian Version / Перейти до української версії](#-українська-версія)**

---

> [!IMPORTANT]
> **Type of README:** 📑 **Academic Coursework Documentation & Explanatory Note README**  
> This file is dedicated specifically to the **Coursework Explanatory Note, Academic Deliverables, and Theoretical Foundations** for the course project *"Development and Efficiency Analysis of Discrete Route Optimization and Autonomous Resource Gathering Algorithms in a Grid Complex (Lethal Company DSA Simulator)"*: explanatory note outline, approved individual assignment, mathematical optimization models, algorithm complexity analyses, and university guidelines.  
> 
> 🎮 **Looking for the Game Simulator & Source Code?**  
> See the dedicated **[Game Application & Source Code README (`../README.md`)](../README.md)** for compilation instructions, gameplay hotkeys, Dear ImGui control panels, real-time metrics, and C++20 architecture.

---

## 🎓 Academic Profile & Metadata

| Metric / Parameter | Value / Description |
| :--- | :--- |
| **Institution** | Vinnytsia National Technical University ([VNTU](https://vntu.edu.ua/)) |
| **Faculty** | Faculty of Information Technology and Computer Engineering (FITCE / ФІТКІ) |
| **Department** | Department of Software Engineering (Кафедра ПЗ) |
| **Academic Program / Major** | F2 Software Engineering (121 Інженерія програмного забезпечення) |
| **Course / Discipline** | Data Structures and Algorithms (Алгоритми та структури даних — АСД) |
| **Author** | **Yaroslav SYCH**, Student of Group 2PI-25b (Сич Ярослав Олександрович) |
| **Academic Advisor / Supervisor** | Ph.D. in Engineering, Associate Professor **Tkachenko O. M.** (Ткаченко О. М.) |
| **Year & Location** | Vinnytsia, 2026 |

---

## 📑 Archive Files & Academic Deliverables

All documentation files located in [`course-projects/DSA-cpp/docs/`](./) are structured as follows:

| File | Format | Language | Description |
| :--- | :---: | :---: | :--- |
| [`Індивідуальне_завдання_Lethal_Company.docx`](./Індивідуальне_завдання_Lethal_Company.docx) | `DOCX` | 🇺🇦 Ukrainian | Approved individual coursework assignment and official specifications |
| [`guidelines-for-coursework-UA.pdf`](./guidelines-for-coursework-UA.pdf) | `PDF` | 🇺🇦 Ukrainian | Official VNTU department methodological guidelines for DSA coursework |
| [`individual_task_variants.docx`](./individual_task_variants.docx) | `DOCX` | 🇺🇦 Ukrainian | Department coursework task variants catalog and assignment register |
| [`notes.md`](./notes.md) | `MD` | 🇺🇦 Ukrainian | Research notes, approved topic formulations, and formal problem statement |
| [`OOP-SychYaroslav-2PI-25B.docx`](./OOP-SychYaroslav-2PI-25B.docx) | `DOCX` | 🇺🇦 Ukrainian | Structural reference template and formatting standard for thesis manuscript |
| [`../benchmark_results.csv`](../benchmark_results.csv) | `CSV` | — | Empirical benchmark dataset comparing all 6 algorithmic combinations |
| [`../report_route.json`](../report_route.json) | `JSON` | — | Exported telemetry, planned waypoints, and step-by-step path coordinates |
| [`../README.md`](../README.md) | `MD` | 🇬🇧 / 🇺🇦 | Root README for the interactive C++20 / SFML 3 game simulation software |

---

## 🎯 Topic & Formal Mathematical Optimization Formulation

### Approved Topic
> **"Development and Efficiency Analysis of Discrete Route Optimization and Autonomous Resource Gathering Algorithms in a Grid Complex (Lethal Company DSA Simulator)"**  
> *(General theme: "Development and efficiency analysis of algorithms and programs")*

### Problem Context & Narrative
Within an abandoned procedural underground industrial complex (modeled as a discrete 2D grid labyrinth with rooms, obstacles, and terrain hazards), a set of $n$ high-value scrap resources $P = \{p_1, p_2, \dots, p_n\}$ is scattered. Each item $p_i$ is defined by:
- Nominal market value $V_i > 0$
- Physical weight $W_i \ge 0$
- Grid coordinates $(x_i, y_i)$

An autonomous worker-agent is deployed at the airlock entry point $S(X_{\text{start}}, Y_{\text{start}})$ and must return safely to the extraction airlock. The agent operates under two critical physical constraints:
1. Maximum backpack payload capacity $C$: $\sum W_i \le C$.
2. Mandatory company quota $Q$: $\sum V_i \ge Q$.
3. Bounded operational energy reserve $E$: total traversal cost of the tour must not exceed $E$.

### Mathematical Formulation of the Optimization Pipeline

The system decomposes the global combinatorial problem into three coupled tiers:

```mermaid
flowchart LR
    subgraph Tier1["1. Resource Selection"]
        direction TB
        K1["0/1 Knapsack DP"]
        K2["Greedy Density"]
    end
    subgraph Tier2["2. Global Tour Planning"]
        direction TB
        T1["Nearest Neighbor TSP"]
        T2["2-Opt Local Search"]
    end
    subgraph Tier3["3. Low-Level Navigation"]
        direction TB
        P1["A* Octile Metric"]
        P2["Dijkstra Search"]
        P3["BFS Wavefront"]
    end
    Tier1 --> Tier2 --> Tier3
```

#### Tier 1: 0/1 Knapsack Optimization (Resource Selection)
Determine a binary decision vector $X = (x_1, x_2, \dots, x_n) \in \{0, 1\}^n$:
$$\max \sum_{i=1}^n V_i \cdot x_i$$
$$\text{subject to } \sum_{i=1}^n W_i \cdot x_i \le C, \quad \sum_{i=1}^n V_i \cdot x_i \ge Q$$

#### Tier 2: Traveling Salesperson Problem (Waypoint Sequencing)
Given selected waypoints $W = \{w_1, w_2, \dots, w_m\}$ with fixed start $w_1 = S$ and return $w_{m+1} = \text{Exit}$:
$$\min_{\sigma} \sum_{j=1}^{m} \text{dist}(w_{\sigma(j)}, w_{\sigma(j+1)})$$
where $\sigma$ is a permutation of waypoint indices with fixed endpoints.

#### Tier 3: 8-Directional Grid Pathfinding
On a weighted grid graph $G = (V, E)$, find a minimal-energy collision-free path $\pi$ between each pair $(w_j, w_{j+1})$:
$$\min_{\pi} \sum_{e \in \pi} \text{cost}(e), \quad \text{subject to } \text{TotalEnergy}(\text{Tour}) \le E$$
where $\text{cost}(e)$ accounts for orthogonal movement ($1.0$), diagonal movement ($\sqrt{2} \approx 1.414$), and terrain penalties (e.g. mud traps $\times 2.5$).

---

## 📖 Structure & Contents of the Explanatory Note

The coursework explanatory note is designed to adhere to DSTU 3008:2015 and VNTU Department of Software Engineering standards:

### 1. Front Matter & Assignment
- Official department title page and signed individual assignment sheet.
- Structured abstract in Ukrainian and English summarizing problem formulation, algorithmic strategies, and empirical findings.

### 2. Introduction (Вступ)
- Industrial relevance of discrete optimization in robotics, warehouse logistics, and autonomous game agents.
- Problem scope: combining 0/1 Knapsack, TSP tour sequencing, and grid graph traversal under strict resource bounds.

### 3. Chapter 1: Problem Statement & Mathematical Modeling (Розділ 1)
- Formal mathematical definitions of the dual-level optimization pipeline.
- Analysis of computational complexity classes (NP-hard status of 0/1 Knapsack and TSP).
- Formal terrain cell model (Floor, Wall, Door, Mud) and Octile heuristic admissibility proof.

### 4. Chapter 2: Algorithmic Architecture & Theoretical Foundations (Розділ 2)
- **Knapsack Solvers:**
  - Dynamic Programming ($O(N \cdot C)$ pseudo-polynomial exact solution) with memory-bounded state table.
  - Greedy Density Heuristic ($O(N \log N)$ approximation) sorting by value-to-weight ratio $V_i / W_i$.
- **TSP Routing Solvers:**
  - Nearest Neighbor construction heuristic ($O(K^2)$).
  - 2-Opt local search refinement ($O(M \cdot K^2)$) using 2-edge exchange operations.
- **Pathfinding Solvers:**
  - Heuristic A\* Search using the Octile distance metric:
    $$h(dx, dy) = 10(dx + dy) - 6\min(dx, dy)$$
  - Uniform-cost Dijkstra Search as an exact ground-truth baseline.
  - Uninformed Breadth-First Search (BFS) for minimal-hop topological routing.
- **Environment Generation:**
  - Recursive Binary Space Partitioning (BSP) generating rooms, corridor connections, and cyclic loops.

### 5. Chapter 3: Software Architecture & Data Structures Implementation (Розділ 3)
- Modern C++20 implementation utilizing a header-only modular architecture in [`include/`](../include/).
- Container complexity mappings:
  - Dynamic grid matrix: `std::vector<std::vector<Cell>>`
  - Priority min-heap: `std::priority_queue<Node, vector<Node>, greater<Node>>`
  - Hash lookup sets: `std::unordered_map` and `std::unordered_set`
  - Backtracking stack: `std::vector<core::Point>` with `std::reverse`
- Real-time hardware-accelerated GUI subsystem: SFML 3.0.2 with Dear ImGui integration.

### 6. Chapter 4: Experimental Performance Analysis & Empirical Benchmarks (Розділ 4)
- Benchmarking methodology across 6 solver configurations ($2 \text{ Knapsack} \times 3 \text{ Pathfinders}$).
- Quantitative comparison metrics:
  - Solver execution runtime ($\mu s$)
  - Memory consumption and dynamic allocations (bytes)
  - Explored node count (search frontier efficiency)
  - Total tour energy cost vs. remaining agent battery
- Optimality gap analysis: DP vs. Greedy value recovery.
- Benchmark data exported directly to [`benchmark_results.csv`](../benchmark_results.csv).

### 7. Conclusions, References & Appendices
- Summary of algorithmic findings and recommendations for real-time robotic and game applications.
- Academic references (Cormen, Knuth, Sedgewick, Hart/Nilsson/Raphael A\* papers).
- Code listings of core algorithmic classes (`DPKnapsack.hpp`, `AStarFinder.hpp`, `TSPSolver.hpp`).

---

## 🔬 Algorithmic Complexity Reference

| Stage | Algorithm | Time Complexity | Space Complexity | Optimality Guarantee |
| :--- | :--- | :---: | :---: | :---: |
| **Knapsack** | **0/1 DP Knapsack** | $O(N \cdot \min(C, \sum W))$ | $O(N \cdot C)$ | **Exact Global Optimum** |
| **Knapsack** | **Greedy Density** | $O(N \log N)$ | $O(N)$ | $1/2$-Approximation (Heuristic) |
| **Routing** | **TSP Nearest Neighbor** | $O(K^2)$ | $O(K)$ | Heuristic ($O(\log K)$ bound) |
| **Routing** | **TSP 2-Opt Local Search**| $O(M \cdot K^2)$ | $O(K)$ | Locally optimal tour |
| **Navigation**| **A\* Pathfinding (Octile)**| $O(E \log V)$ | $O(V)$ | **Optimal (Admissible metric)** |
| **Navigation**| **Dijkstra Algorithm** | $O(E \log V)$ | $O(V)$ | **Optimal (Uniform-cost)** |
| **Navigation**| **BFS (Wavefront)** | $O(V + E)$ | $O(V)$ | Optimal in hop count only |
| **Topology** | **BSP Generator** | $O(R \log R)$ | $O(W \cdot H)$ | Procedural guarantee |

---

## 📐 Methodological & Formatting Standards (VNTU)

The explanatory note follows VNTU coursework standards:
- **Paper Format:** A4, portrait orientation.
- **Margins:** Left — 30 mm, Right — 10 mm, Top — 20 mm, Bottom — 20 mm.
- **Font & Spacing:** Times New Roman, 14 pt, 1.5 line spacing.
- **Formulas:** Centered, numbered right-aligned within chapter notation: e.g., `(2.1)`.
- **Figures & Tables:** Captioned, referenced in text before placement.
- **Standards:** DSTU 3008:2015 (Звіти у сфері науки і техніки) and DSTU 8302:2015 (Бібліографічне посилання).

---

<br />

---

<a id="-українська-версія"></a>

# 🇺🇦 Пояснювальна записка до курсової роботи та академічна документація

> 🌐 **Мова:** Українська &nbsp;|&nbsp; **[🇬🇧 Switch to English Version / Перейти до англійської версії](#-english-version)**

---

> [!IMPORTANT]
> **Тип README-файлу:** 📑 **Документація до пояснювальної записки та матеріалів курсової роботи**  
> Цей файл присвячено виключно **пояснювальній записці, академічним матеріалам та теоретичному обґрунтуванню** курсового проєкту з дисципліни «Алгоритми та структури даних» на тему: *«Розробка та дослідження ефективності алгоритмів дискретної оптимізації маршруту та збору ресурсів у сітковому лабіринті (Симулятор Lethal Company DSA)»*: структурі пояснювальної записки, затвердженому індивідуальному завданню, формальній математичній моделі оптимізації, теоретичній оцінці складності алгоритмів та методичним вимогам кафедри ВНТУ.  
> 
> 🎮 **Шукаєте гру-симулятор, вихідний код та інструкції зі збірки?**  
> Перейдіть до головного **[README ігрового застосунку (`../README.md`)](../README.md)**, де описано збірку через CMake, керування у грі, аналітичний HUD на Dear ImGui та архітектуру C++20.

---

## 🎓 Академічний профіль та метадані

| Параметр / Показник | Значення / Опис |
| :--- | :--- |
| **Заклад вищої освіти** | Вінницький національний технічний університет ([ВНТУ](https://vntu.edu.ua/)) |
| **Факультет** | Факультет інформаційних технологій та комп'ютерної інженерії (ФІТКІ) |
| **Кафедра** | Кафедра програмної інженерії (ПЗ) |
| **Спеціальність** | F2 «Інженерія програмного забезпечення» (121) |
| **Дисципліна** | «Алгоритми та структури даних» (АСД) |
| **Виконавець** | студент групи 2ПІ-25б, **Сич Ярослав Олександрович** |
| **Керівник проєкту** | к.т.н., доцент кафедри ПЗ **Ткаченко О. М.** |
| **Рік та місто** | Вінниця, 2026 |

---

## 📑 Склад архіву та академічні матеріали

Усі матеріали каталогу [`course-projects/DSA-cpp/docs/`](./) структуровані наступним чином:

| Файл / Каталог | Формат | Мова | Призначення та зміст |
| :--- | :---: | :---: | :--- |
| [`Індивідуальне_завдання_Lethal_Company.docx`](./Індивідуальне_завдання_Lethal_Company.docx) | `DOCX` | 🇺🇦 Українська | Офіційне затверджене індивідуальне завдання до курсової роботи |
| [`guidelines-for-coursework-UA.pdf`](./guidelines-for-coursework-UA.pdf) | `PDF` | 🇺🇦 Українська | Офіційні методичні вказівки кафедри ПЗ ВНТУ до виконання курсових робіт з АСД |
| [`individual_task_variants.docx`](./individual_task_variants.docx) | `DOCX` | 🇺🇦 Українська | Збірник варіантів завдань кафедри та реєстр тем курсових робіт |
| [`notes.md`](./notes.md) | `MD` | 🇺🇦 Українська | Дослідницькі нотатки, варіанти теми та формальна постановка оптимізаційної задачі |
| [`OOP-SychYaroslav-2PI-25B.docx`](./OOP-SychYaroslav-2PI-25B.docx) | `DOCX` | 🇺🇦 Українська | Зразок оформлення та шаблон рукопису записки за стандартами ВНТУ |
| [`../benchmark_results.csv`](../benchmark_results.csv) | `CSV` | — | Масив емпіричних даних порівняльного бенчмаркінгу 6 конфігурацій алгоритмів |
| [`../report_route.json`](../report_route.json) | `JSON` | — | Експортований файл телеметрії маршруту, опорних точок та координат кроків |
| [`../README.md`](../README.md) | `MD` | 🇬🇧 / 🇺🇦 | Головний README з описом програми, симулятора та інструкціями зі збірки |

---

## 🎯 Тема та математична постановка комплексної задачі оптимізації

### Затверджена тема
> **«Розробка та дослідження ефективності алгоритмів дискретної оптимізації маршруту та збору ресурсів у сітковому лабіринті (Симулятор Lethal Company DSA)»**  
> *(Загальна тема: «Розробка й аналіз ефективності алгоритмів і програм»)*

### Сюжетний контекст та змістовна постановка
У покинутому підземному промисловому лабіринті розташовано множину $n$ цінних ресурсів (металобрухту) $P = \{p_1, p_2, \dots, p_n\}$. Кожен ресурс $p_i$ має:
- Ринкову вартість $V_i > 0$
- Фізичну вагу $W_i \ge 0$
- Координати в сітці лабіринту $(x_i, y_i)$

Автономний агент-робітник висаджується у точці шлюзу $S(X_{\text{start}}, Y_{\text{start}})$ і має повернутися до шлюзу евакуації. Діють обмеження:
1. Вантажопідйомність рюкзака $C$: $\sum W_i \le C$.
2. Мінімальна обов'язкова квота вартості $Q$: $\sum V_i \ge Q$.
3. Лімітований запас ходової енергії агента $E$: сумарна вартість пройденого шляху $\le E$.

### Математична модель трирівневого пайплайну оптимізації

```mermaid
flowchart LR
    subgraph Рівень1["1. Оптимізація вибору ресурсів"]
        direction TB
        K1["0/1 DP Knapsack"]
        K2["Greedy Knapsack"]
    end
    subgraph Рівень2["2. Планування туру обходу"]
        direction TB
        T1["Nearest Neighbor TSP"]
        T2["Локальний пошук 2-Opt"]
    end
    subgraph Рівень3["3. Траєкторна навігація в сітці"]
        direction TB
        P1["A* Октагональний"]
        P2["Пошук Дейкстри"]
        P3["Хвильовий BFS"]
    end
    Рівень1 --> Рівень2 --> Рівень3
```

#### Рівень 1: Оптимізація вибору предметів (Задача про рюкзак 0/1)
Знайти двійковий вектор вибору $X = (x_1, x_2, \dots, x_n) \in \{0, 1\}^n$:
$$\max \sum_{i=1}^n V_i \cdot x_i$$
$$\text{за умов: } \sum_{i=1}^n W_i \cdot x_i \le C, \quad \sum_{i=1}^n V_i \cdot x_i \ge Q$$

#### Рівень 2: Оптимізація черговості обходу точок (Задача комівояжера TSP)
Для обраних цільових точок $W = \{w_1, w_2, \dots, w_m\}$ із закріпленим стартом $w_1 = S$ та фінішем $w_{m+1} = \text{Exit}$:
$$\min_{\sigma} \sum_{j=1}^{m} \text{dist}(w_{\sigma(j)}, w_{\sigma(j+1)})$$
де $\sigma$ — перестановка індексів проміжних точок.

#### Рівень 3: Пошук шляху на 8-зв'язному зваженому графі
На графі сітки $G = (V, E)$ знайти найменш енергозатратну траєкторію $\pi$ між кожною парою точок $(w_j, w_{j+1})$:
$$\min_{\pi} \sum_{e \in \pi} \text{cost}(e), \quad \text{за умови: } \text{TotalEnergy}(\text{Tour}) \le E$$
де $\text{cost}(e)$ враховує ортогональний крок ($1.0$), діагональний перехід ($\approx 1.414$) та штрафи за пересування болотом ($\times 2.5$).

---

## 📖 Структура та зміст пояснювальної записки до курсової роботи

Пояснювальна записка структурована відповідно до вимог ДСТУ 3008:2015 та методичних вказівок кафедри ПЗ ВНТУ:

### 1. Вступна частина
- Титульний аркуш та підписане індивідуальне завдання.
- Анотація українською та англійською мовами із зазначенням обсягу, кількості таблиць, ілюстрацій, джерел та результатів.

### 2. Вступ
- Актуальність задач комбінаторної оптимізації у логістиці, автономній робототехніці та штучному інтелекті ігор.
- Мета і завдання курсової роботи: дослідження ефективності точних та наближених алгоритмів.

### 3. Розділ 1: Постановка задачі та математичне моделювання
- Детальний аналіз предметної області та формалізація трирівневої моделі.
- Класифікація складності задач (NP-повні задачі Knapsack та TSP).
- Обґрунтування допустимості октагональної евристики для сітки з діагональними переходами.

### 4. Розділ 2: Дослідження та проєктування алгоритмічного забезпечення
- **Алгоритми задачі про рюкзак:**
  - Динамічне програмування ($O(N \cdot C)$) із мемоізацією матриці та захистом від переповнення пам'яті.
  - Жадібний евристичний алгоритм за питомою цінністю ($O(N \log N)$).
- **Алгоритми задачі комівояжера (TSP):**
  - Метод найближчого сусіда ($O(K^2)$) для швидкої жадібної побудови базового маршруту.
  - Локальний пошук 2-Opt ($O(M \cdot K^2)$) з оптимізацією схрещених ребер.
- **Алгоритми навігації в лабіринті:**
  - Алгоритм A\* з октагональною евристикою $h = 10(dx+dy) - 6\min(dx,dy)$ та чергою з пріоритетом.
  - Алгоритм Дейкстри як точний еталон вартості шляху.
  - Пошук у ширину (BFS) для топологічного аналізу найменшої кількості кроків.
- **Процедурна генерація лабіринту:**
  - Алгоритм двійкового розбиття простору (BSP) з гарантованою досяжністю кімнат і циклічними зв'язками.

### 5. Розділ 3: Програмна реалізація та структури даних
- Реалізація стандартом C++20 із модульною архітектурою в папці [`include/`](../include/).
- Відображення абстрактних структур на контейнери STL:
  - Сітка лабіринту: `std::vector<std::vector<Cell>>`
  - Пріоритетна черга: `std::priority_queue<Node, vector<Node>, greater<Node>>`
  - Хеш-таблиці та множини: `std::unordered_map`, `std::unordered_set`
  - Стек реверсу шляху: `std::vector<core::Point>` із `std::reverse`
- Графічна підсистема на базі SFML 3.0.2 та аналітичний HUD Dear ImGui.

### 6. Розділ 4: Експериментальне дослідження та аналіз ефективності
- Методологія автоматизованого бенчмаркінгу 6 комбінацій алгоритмів ($2 \text{ рюкзаки} \times 3 \text{ шукачі шляху}$).
- Порівняння за метриками:
  - Час обчислення ($\mu s$)
  - Використана пам'ять та динамічні алокації (байти)
  - Кількість розкритих вузлів у пошуковому графі
  - Загальні енерговитрати маршруту та залишок заряду агента
- Порівняльний аналіз похибки жадібного рюкзака відносно точного оптимуму DP.
- Експорт результатів у файл [`benchmark_results.csv`](../benchmark_results.csv).

### 7. Висновки, список джерел та додатки
- Науково-практичні рекомендації щодо вибору алгоритмів для високонавантажених ігрових рушіїв.
- Список використаних літературних джерел (Кормен, Кнут, Седжвік, стандарти ISO C++).
- Додатки з вихідним кодом ключових класів та даними бенчмарків.

---

## 🔬 Порівняльна таблиця складності алгоритмів

| Етап оптимізації | Алгоритм | Часова складність | Просторова складність | Гарантія оптимальності |
| :--- | :--- | :---: | :---: | :---: |
| **Вибір брухту** | **0/1 Knapsack DP** | $O(N \cdot \min(C, \sum W))$ | $O(N \cdot C)$ | **Точний глобальний оптимум** |
| **Вибір брухту** | **Greedy Knapsack** | $O(N \log N)$ | $O(N)$ | $1/2$-наближення (евристика) |
| **Маршрут TSP** | **TSP Nearest Neighbor** | $O(K^2)$ | $O(K)$ | Евристичний ланцюжок |
| **Маршрут TSP** | **TSP 2-Opt Local Search**| $O(M \cdot K^2)$ | $O(K)$ | Локально оптимальний тур |
| **Пошук шляху** | **A\* з октагональною метрикою** | $O(E \log V)$ | $O(V)$ | **Оптимальний (допустима евристика)** |
| **Пошук шляху** | **Алгоритм Дейкстри** | $O(E \log V)$ | $O(V)$ | **Оптимальний за вартістю** |
| **Пошук шляху** | **Пошук у ширину (BFS)** | $O(V + E)$ | $O(V)$ | Оптимальний лише за кількістю кроків |
| **Топологія** | **Генератор BSP** | $O(R \log R)$ | $O(W \cdot H)$ | Гарантована досяжність |

---

## 📐 Методичні вимоги та стандарти оформлення (ВНТУ)

Пояснювальна записка відповідає стандартам ВНТУ:
- **Формат аркуша:** А4, книжкова орієнтація.
- **Поля:** ліве — 30 мм, праве — 10 мм, верхнє — 20 мм, нижнє — 20 мм.
- **Шрифт та інтервал:** Times New Roman, 14 pt, полуторний інтервал (1.5).
- **Формули:** центрування формули з наскрізною нумерацією у розділі біля правого поля: наприклад, `(2.1)`.
- **Ілюстрації та таблиці:** обов'язкове посилання в тексті перед розміщенням об'єкта.
- **Державні стандарти:** ДСТУ 3008:2015 (Звіти у сфері науки і техніки) та ДСТУ 8302:2015 (Бібліографічне посилання).
