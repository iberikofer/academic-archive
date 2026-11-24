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

| File / Directory | Format | Language | Description |
| :--- | :---: | :---: | :--- |
| [`DSA-Sych-Yaroslav-2PI25b.docx`](./DSA-Sych-Yaroslav-2PI25b.docx) | `DOCX` | 🇺🇦 Ukrainian | **Final official coursework explanatory note manuscript** (Title page, approved assignment, abstract, Chapters 1–3, empirical tables 3.1–3.3, figures 3.1–3.2, conclusions, literature, and Appendix A code) |
| [`DSA-Sych-Yaroslav-2PI25b.pdf`](./DSA-Sych-Yaroslav-2PI25b.pdf) | `PDF` | 🇺🇦 Ukrainian | **Print-ready compiled PDF version** of the explanatory note conforming to DSTU 3008:2015 |
| [`guidelines-for-coursework-UA.pdf`](./guidelines-for-coursework-UA.pdf) | `PDF` | 🇺🇦 Ukrainian | Official VNTU department methodological guidelines for DSA coursework |
| [`assets/`](./assets/) | `DIR` | 🇺🇦 / — | Academic figure assets ([`3.1.png`](./assets/3.1.png), [`3.2.png`](./assets/3.2.png)) and department assignment register ([`individual_task_variants.docx`](./assets/individual_task_variants.docx), [`individual_task_variants.pdf`](./assets/individual_task_variants.pdf)) |
| [`../benchmark_results.csv`](../benchmark_results.csv) | `CSV` | — | Empirical benchmark dataset comparing all 6 algorithmic combinations |
| [`../input_config.json`](../input_config.json) | `JSON` | — | Declarative mission configuration file (agent capacities, quotas, BSP labyrinth dimensions, algorithmic selections) |
| `session_state.json` | `JSON` | — | Local runtime session snapshot (active maze, telemetry, GUI window layouts, zoom, language preferences) |
| `report_route.json` | `JSON` | — | Exported telemetry, planned waypoints, and step-by-step path coordinates (generated on-demand via simulator UI) |
| [`../README.md`](../README.md) | `MD` | 🇬🇧 / 🇺🇦 | Root README for the interactive C++20 / SFML 3 game simulation software |

### ⚙️ Simulator I/O & JSON State Specifications

The software relies on structured JSON formats for declarative inputs, runtime session state, and analytical reporting:

1. **`input_config.json` (Declarative Mission Configuration)**:
   - Configures the simulation parameters before runtime or when loading preset scenarios.
   - **`agent`**: Sets backpack payload capacity (`backpack_capacity_kg`), company quota targets (`company_quota_credits`), battery energy reserve (`initial_energy`), energy limit enforcement (`energy_limit_enabled`), and traversal mode (`route_mode`: `"PointAToPointB"` vs `"RoundTrip"`).
   - **`algorithms`**: Selects the active Knapsack optimizer (0/1 DP vs Greedy Value/Weight) and Pathfinding algorithm (A* Octile, Dijkstra Uniform-Cost, or BFS Shortest-Path).
   - **`map`**: Governs procedural BSP generation: grid dimensions (`width`, `height`), room limits (`min_room_size`, `max_room_size`), obstacle generation (`mud_spawn_chance`, `extra_passage_chance`), and deterministic RNG seed (`seed`).

2. **`session_state.json` (Full Exploration State & UI Persistence)**:
   - Provides complete lossless serialization of the active game session and player preferences.
   - **`config`**: Synchronized simulation parameters and display flags (`show_grid`, `show_optimal_route`, `show_walked_cells`, `camera_follow_agent`, `vsync_enabled`).
   - **`gui`**: Preserves tactical UI geometry across restarts: sidebar width and collapse state (`sidebar_width`, `sidebar_collapsed`), mission status panel dimensions (`mission_panel_width`, `mission_panel_collapsed`), camera coordinates and zoom level (`camera_zoom`, `camera_center_x`, `camera_center_y`), accordion section states, fullscreen toggle, and global language preference (`"language"`: `"UA"` or `"EN"`).
   - **`maze`**: Exact spatial map snapshot: generated room bounds, scrap item locations with weights and values, and the discrete tile matrix (walkable paths, mud hazards, solid walls).
   - **`agent`**: Dynamic traversal state: worker coordinates $(X, Y)$, battery energy, list of collected scrap IDs, executed step history, and mission status.
   - **`benchmarks`**: In-memory matrix of benchmark metrics recorded across algorithmic combinations for the current session.

3. **`report_route.json` (Analytical Route Telemetry Export)**:
   - Exported on-demand via the simulator interface (`↓ Export report_route.json`) for empirical verification, containing calculated TSP waypoint sequences, step-by-step path coordinates, and algorithmic benchmark logs.

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

### 5. Chapter 3: Software Implementation, User Interface & Experimental Analysis (Розділ 3)
- Modern C++20 implementation utilizing a modular architecture in [`include/`](../include/).
- Container complexity mappings:
  - Dynamic grid matrix: `std::vector<std::vector<Cell>>`
  - Priority min-heap: `std::priority_queue<Node, vector<Node>, greater<Node>>`
  - Hash lookup sets: `std::unordered_map` and `std::unordered_set`
  - Backtracking stack: `std::vector<core::Point>` with `std::reverse`
- Real-time hardware-accelerated GUI subsystem: SFML 3.0.2 with Dear ImGui integration and bilingual localization (UA / EN).
- Section 3.2: Architecture and User Interface (Figure 3.1).
- Section 3.3: Experimental Performance Analysis & Empirical Benchmarks across 6 solver configurations:
  - Table 3.1: Pathfinding algorithms comparison ($A^*$ vs. Dijkstra vs. BFS on $30 \times 30$ grid).
  - Table 3.2: Knapsack optimization comparison ($0/1$ Exact DP vs. Greedy Density Heuristic).
  - Table 3.3: TSP route optimization parameters (Nearest Neighbor + 2-Opt local search with Multi-Target Dijkstra distance matrix cache ~160x acceleration).
  - Figure 3.2: Interactive in-game benchmark and comparative analysis window.
- Section 3.4: Spatial Complexity Analysis (Memory Footprint formula $(N+1)(C+1)\cdot \text{sizeof(int)}$ and Heap Chunk Pooling).
- Benchmark telemetry exported to [`benchmark_results.csv`](../benchmark_results.csv) and [`report_route.json`](../report_route.json).

### 6. Conclusions, References & Appendices
- Summary of algorithmic findings and recommendations for real-time robotics and game engines.
- Academic references (Cormen, Knuth, Sedgewick, Hart/Nilsson/Raphael $A^*$ papers, ISO C++ standards).
- Appendix A: Complete C++ source code listings of core algorithmic classes (`DPKnapsack.hpp`, `AStarFinder.hpp`, `TSPSolver.hpp`).

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
| [`DSA-Sych-Yaroslav-2PI25b.docx`](./DSA-Sych-Yaroslav-2PI25b.docx) | `DOCX` | 🇺🇦 Українська | **Офіційний фінальний рукопис пояснювальної записки** (Титульний аркуш, індивідуальне завдання, анотація, Розділи 1–3, емпіричні таблиці 3.1–3.3, рисунки 3.1–3.2, висновки, джерела та Додаток А) |
| [`DSA-Sych-Yaroslav-2PI25b.pdf`](./DSA-Sych-Yaroslav-2PI25b.pdf) | `PDF` | 🇺🇦 Українська | **Готовий до друку та перевірки PDF-файл** пояснювальної записки за ДСТУ 3008:2015 |
| [`guidelines-for-coursework-UA.pdf`](./guidelines-for-coursework-UA.pdf) | `PDF` | 🇺🇦 Українська | Офіційні методичні вказівки кафедри ПЗ ВНТУ до виконання курсових робіт з АСД |
| [`assets/`](./assets/) | `DIR` | 🇺🇦 / — | Графічні матеріали ([`3.1.png`](./assets/3.1.png), [`3.2.png`](./assets/3.2.png)) та реєстр завдань кафедри ([`individual_task_variants.docx`](./assets/individual_task_variants.docx), [`individual_task_variants.pdf`](./assets/individual_task_variants.pdf)) |
| [`../benchmark_results.csv`](../benchmark_results.csv) | `CSV` | — | Масив емпіричних даних порівняльного бенчмаркінгу 6 конфігурацій алгоритмів |
| [`../input_config.json`](../input_config.json) | `JSON` | — | Декларативний конфігураційний файл місії (параметри робітника, квота, розміри BSP-лабіринту, алгоритми) |
| `session_state.json` | `JSON` | — | Знімок поточної сесії симулятора (активний лабіринт, телеметрія, геометрія вікон GUI, зум, мова) |
| `report_route.json` | `JSON` | — | Експортований файл телеметрії маршруту, опорних точок та координат кроків (формується через інтерфейс) |
| [`../README.md`](../README.md) | `MD` | 🇬🇧 / 🇺🇦 | Головний README з описом програми, симулятора та інструкціями зі збірки |

### ⚙️ Специфікація JSON-файлів конфігурації та збереження сесії

Програмний комплекс використовує структуровані JSON-файли для декларативної конфігурації, збереження поточної сесії та аналітичної звітності:

1. **`input_config.json` (Декларативна конфігурація місії)**:
   - Задає вхідні параметри сценарію перед запуском симулятора або завантаженням попередньо налаштованих шаблонів.
   - **`agent`**: Місткість рюкзака (`backpack_capacity_kg`), норма обов'язкової квоти компанії (`company_quota_credits`), початковий запас енергії (`initial_energy`), прапорець обмеження енергії (`energy_limit_enabled`) та режим руху (`route_mode`: `"PointAToPointB"` або `"RoundTrip"`).
   - **`algorithms`**: Вибір методів оптимізації рюкзака (0/1 DP або жадібний за питомою цінністю) та пошуку шляху (A* Octile, Dijkstra Uniform-Cost, BFS).
   - **`map`**: Параметри процедурної BSP-генерації: габарити сітки (`width`, `height`), діапазон кімнат (`min_room_size`, `max_room_size`), ймовірність перешкод (`mud_spawn_chance`, `extra_passage_chance`) та числовий зерно генератора (`seed`).

2. **`session_state.json` (Повний знімок сесії та збереження інтерфейсу)**:
   - Забезпечує безвтратну серіалізацію активного стану комплексу з можливістю продовження експедиції (`has_active_session`).
   - **`config`**: Синхронізовані параметри симуляції та прапорці відображення (`show_grid`, `show_optimal_route`, `show_walked_cells`, `camera_follow_agent`, `vsync_enabled`).
   - **`gui`**: Збереження геометрії інтерфейсу між перезапусками: ширина та згортання бічної панелі (`sidebar_width`, `sidebar_collapsed`), параметри панелі місії (`mission_panel_width`, `mission_panel_collapsed`), координати фокусу та рівень масштабу камери (`camera_zoom`, `camera_center_x`, `camera_center_y`), стани акордеонів, повноекранний режим і мова інтерфейсу (`"language"`: `"UA"` або `"EN"`).
   - **`maze`**: Повний стан мапи: згенеровані кімнати, позиції металобрухту з їхньою вагою і цінністю, матриця клітинок (прохідні, багнюка, монолітні стіни).
   - **`agent`**: Динамічний стан агента: поточні координати $(X, Y)$, залишок заряду батареї, список зібраних предметів, історія пройдених кроків і статус виконання місії.
   - **`benchmarks`**: Таблиця результатів обчислювальної складності всіх 6 комбінацій алгоритмів для поточної конфігурації.

3. **`report_route.json` (Аналітичний звіт телеметрії маршруту)**:
   - Формується на вимогу користувача через інтерфейс програми (`↓ Експортувати report_route.json`) для науково-дослідного аналізу: містить розраховану послідовність опорних точок TSP, порядкові координати кожного кроку та порівняльні бенчмарки.

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

### 5. Розділ 3: Програмна реалізація, інтерфейс та аналіз ефективності алгоритмів
- Реалізація стандартом C++20 із модульною архітектурою в каталозі [`include/`](../include/).
- Відображення абстрактних структур на контейнери STL:
  - Сітка лабіринту: `std::vector<std::vector<Cell>>`
  - Пріоритетна черга: `std::priority_queue<Node, vector<Node>, greater<Node>>`
  - Хеш-таблиці та множини: `std::unordered_map`, `std::unordered_set`
  - Стек реверсу шляху: `std::vector<core::Point>` із `std::reverse`
- Графічна підсистема на базі SFML 3.0.2 та аналітичний HUD Dear ImGui з підтримкою двомовної локалізації (UA / EN).
- Підрозділ 3.2: Архітектура програмного забезпечення та графічний інтерфейс (Рисунок 3.1).
- Підрозділ 3.3: Експериментальне дослідження та емпіричний бенчмаркінг 6 конфігурацій алгоритмів:
  - Таблиця 3.1: Порівняння алгоритмів пошуку шляху ($A^*$ проти Дейкстри та BFS на сітці $30 \times 30$).
  - Таблиця 3.2: Дослідження оптимізації рюкзака (Точний $0/1$ DP проти жадібного за питомою цінністю).
  - Таблиця 3.3: Параметри маршрутизації TSP (Найближчий сусід + 2-Opt локальний пошук із кешуванням матриці відстаней Multi-Target Dijkstra — прискорення ~160x).
  - Рисунок 3.2: Інтерактивне вікно зведеного порівняльного аналізу алгоритмів.
- Підрозділ 3.4: Аналіз просторової складності (формула оцінки пам'яті $(N+1)(C+1)\cdot \text{sizeof(int)}$ та пулінг пам'яті Heap Chunk Pooling).
- Експорт результатів у файли [`benchmark_results.csv`](../benchmark_results.csv) та [`report_route.json`](../report_route.json).

### 6. Висновки, список використаних джерел та додатки
- Науково-практичні рекомендації щодо вибору алгоритмів для високонавантажених ігрових рушіїв.
- Список використаних літературних джерел (Кормен, Кнут, Седжвік, стандарти ISO C++).
- Додаток А: Повний вихідний код ключових класів та алгоритмів (`DPKnapsack.hpp`, `AStarFinder.hpp`, `TSPSolver.hpp`).

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
