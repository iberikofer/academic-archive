<a id="-english-version"></a>

# 🇬🇧 English Version

> 🌐 **Language:** English &nbsp;|&nbsp; **[🇺🇦 Switch to Ukrainian Version / Перейти до української версії](#-українська-версія)**

> [!IMPORTANT]
> **Type of README:** 🎮 **Game Application / Simulation Software README**  
> This file is dedicated specifically to the **Lethal Company DSA Simulator** software application: combinatorial algorithms pipeline (0/1 Knapsack, TSP with 2-Opt, A\*/Dijkstra/BFS pathfinding, BSP maze generation), Dear ImGui HUD, interactive controls, C++20 / SFML 3 architecture, and build/run instructions.
>
> 📑 **Looking for the Coursework Explanatory Note & Academic Documentation?**  
> See the dedicated **[Coursework Explanatory Note & Documentation README (`docs/README.md`)](./docs/README.md)** for the academic report structure, approved individual assignment, mathematical problem formulations, guideline compliance, and algorithmic research notes.

---

## 📑 Project Documentation & Guidelines

| Document                            | Format | Description & Link                                                                                                                                                  |
| :---------------------------------- | :----: | :------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **Coursework Documentation README** |  `MD`  | 📑 [docs/README.md](./docs/README.md) — Comprehensive guide to coursework explanatory note, mathematical model & guidelines                                         |
| **Coursework Thesis (DOCX)**        | `DOCX` | 🇺🇦 [`DSA-Sych-Yaroslav-2PI25b.docx`](./docs/DSA-Sych-Yaroslav-2PI25b.docx) — Final official coursework explanatory note manuscript (DSTU 3008:2015)                |
| **Coursework Thesis (PDF)**         | `PDF`  | 🇺🇦 [`DSA-Sych-Yaroslav-2PI25b.pdf`](./docs/DSA-Sych-Yaroslav-2PI25b.pdf) — Print-ready compiled PDF version of the thesis note                                    |
| **Methodological Guidelines**       | `PDF`  | 🇺🇦 [`guidelines-for-coursework-UA.pdf`](./docs/guidelines-for-coursework-UA.pdf) — Official VNTU DSA coursework requirements and standards                          |
| **Task Variants & Figures**         | `DIR`  | 🇺🇦 [`docs/assets/`](./docs/assets/) — Department coursework task variants list and thesis figure assets                                                            |

---

## 🎓 Academic Profile & Metadata

- **Institution:** Vinnytsia National Technical University ([VNTU](https://vntu.edu.ua/))
- **Faculty:** Faculty of Information Technology and Computer Engineering (FITCE)
- **Department:** Department of Software Engineering
- **Program / Major:** F2 Software Engineering
- **Course / Discipline:** Data Structures and Algorithms (DSA)
- **Author:** Yaroslav SYCH, Student of Group 2PI-25b
- **Academic Advisor / Supervisor:** Ph.D. in Engineering, Associate Professor Tkachenko O. M.
- **Year & Location:** Vinnytsia, 2026

---

## 🎯 Topic & Project Objectives

### Project Topic

> **"Development and Efficiency Analysis of Discrete Route Optimization and Autonomous Resource Gathering Algorithms in a Grid Complex (Lethal Company DSA Simulator)"**
> _(General theme: "Development and efficiency analysis of algorithms and programs")_

### Background & Gamification Concept

The project simulates an autonomous worker-agent expedition inspired by the popular cooperative game **Lethal Company**. An employee of "The Company" is deployed into an abandoned procedural underground industrial complex to collect high-value scrap items scattered across hazardous terrain, meeting a strict profit quota while operating under bounded energy constraints and backpack payload capacity.

Solving this challenge requires a multi-tier combinatorial optimization pipeline:

1. **Resource Value Maximization:** Solving the 0/1 Knapsack problem to select the optimal subset of scrap items subject to physical weight limits.
2. **Global Tour Planning:** Solving the Traveling Salesperson Problem (TSP) with fixed departure (Airlock) and return/extraction points to determine the optimal sequence of waypoints.
3. **Low-Level Grid Navigation:** Finding energy-efficient, collision-free paths between consecutive waypoints in an 8-directional weighted maze containing mud traps, obstacles, and doorway bottlenecks.

### Key Objectives

1. Design a formal mathematical formulation for the dual-level combinatorial optimization problem.
2. Implement and contrast alternative algorithmic strategies:
   - **0/1 Knapsack Problem:** Exact Dynamic Programming ($O(N \cdot C)$) vs. Heuristic Greedy Density approach ($O(N \log N)$).
   - **Pathfinding on Weighted Graphs:** Heuristic A\* (with Octile distance metric) vs. Dijkstra uniform-cost search vs. Uninformed Breadth-First Search (BFS).
   - **Routing / TSP:** Nearest Neighbor heuristic coupled with 2-Opt local search optimization.
3. Procedurally generate realistic multi-room industrial layouts using **Binary Space Partitioning (BSP)** with guaranteed room reachability, cyclic corridors, and configurable hazard density.
4. Construct an interactive real-time 2D simulation application in **C++20** using **SFML 3** for hardware-accelerated rendering and **Dear ImGui** for developer analytics and control HUD.
5. Provide automatic profiling metrics: execution runtime ($\mu s$), memory consumption (bytes), nodes explored, total path length, total collected scrap value, and energy expenditure.
6. Support batch benchmarking and CSV data export for empirical algorithmic analysis.

---

## 🧩 Algorithmic Architecture & Data Structures

```mermaid
flowchart TD
    A[Start Expedition / New Seed] --> B[BSP Maze Generator]
    B --> C[Spawn Scrap Items & Airlock/Exit]
    C --> D{Knapsack Solver}
    D -->|Exact| D1[0/1 DP Knapsack]
    D -->|Heuristic| D2[Greedy Density Knapsack]
    D1 --> E[Selected Target Scrap Items]
    D2 --> E
    E --> F[TSP Route Planner]
    F --> F1[Nearest Neighbor Tour]
    F1 --> F2[2-Opt Local Search]
    F2 --> G[Waypoints Sequence]
    G --> H{Low-Level Pathfinding}
    H -->|A* Octile| H1[AStarFinder]
    H -->|Dijkstra| H2[DijkstraFinder]
    H -->|BFS| H3[BFSFinder]
    H1 --> I[Stitched Collision-Free Route]
    H2 --> I
    H3 --> I
    I --> J[SimulationEngine / SFML + ImGui Viewport]
```

### 1. Optimization Pipeline

| Stage              | Algorithm                |           Complexity (Time / Space)            | Implementation Details                                                                                               |
| :----------------- | :----------------------- | :--------------------------------------------: | :------------------------------------------------------------------------------------------------------------------- |
| **Item Selection** | **0/1 Knapsack DP**      | $O(N \cdot \min(C, \sum W)) \ /\ O(N \cdot C)$ | Exact optimal solution; memory-bounded allocation with fast-path bypass when total weight $\le$ capacity.            |
| **Item Selection** | **Greedy Knapsack**      |            $O(N \log N) \ /\ O(N)$             | Fast heuristic sorting by value density $V_i / W_i$; handles zero-weight items via infinity density.                 |
| **Global Tour**    | **TSP Nearest Neighbor** |               $O(K^2) \ /\ O(K)$               | Greedy order construction between $K$ waypoints with fixed start airlock and return/exit destination.                |
| **Global Tour**    | **TSP 2-Opt Search**     |           $O(M \cdot K^2) \ /\ O(K)$           | Local 2-edge exchange optimization; preserves fixed endpoints and handles disconnected graphs safely.                |
| **Navigation**     | **A\* Pathfinding**      |            $O(E \log V) \ /\ O(V)$             | 8-directional search with Octile metric $h = 10(dx+dy) - 6\min(dx,dy)$; uses `closedSet` to prune stale queue items. |
| **Navigation**     | **Dijkstra Finder**      |            $O(E \log V) \ /\ O(V)$             | Uniform-cost search exploring equidistant contours; serves as the exact ground truth for path costs.                 |
| **Navigation**     | **BFS Finder**           |              $O(V + E) \ /\ O(V)$              | Uninformed 8-directional wave flood fill; minimizes hop count regardless of terrain energy costs.                    |
| **Environment**    | **BSP Generator**        |        $O(R \log R) \ /\ O(W \cdot H)$         | Recursive Binary Space Partitioning; generates industrial rooms, hallways, and extra cyclic loops.                   |

### 2. Standard Data Structures Mapping

| Abstract Data Structure       | C++ Standard Type                                        | Role in the System                                                                 |
| :---------------------------- | :------------------------------------------------------- | :--------------------------------------------------------------------------------- |
| **Dynamic 2D Array**          | `std::vector<std::vector<Cell>>`                         | Discrete grid matrix storing floor, wall, doorway, and mud cell types.             |
| **Priority Queue / Min-Heap** | `std::priority_queue<Node, vector<Node>, greater<Node>>` | Open set for A\* and Dijkstra frontier expansion with $O(\log N)$ extraction.      |
| **FIFO Queue**                | `std::queue<core::Point>`                                | Wavefront exploration buffer for Breadth-First Search (BFS).                       |
| **Hash Tables / Sets**        | `std::unordered_map`, `std::unordered_set`               | Fast $O(1)$ lookups for `gScore`, `cameFrom` parent maps, and closed visited sets. |
| **Matrix DP Table**           | `std::vector<std::vector<int>>`                          | 2D dynamic programming memoization table for knapsack values.                      |
| **Call Stack / Reversal**     | `std::vector<core::Point>` with `std::reverse`           | Backtracking path reconstruction from destination back to source.                  |

---

## 🎮 Interactive Simulation & GUI Features

- **Procedural 2D Grid Engine:** Hardware-accelerated rendering using **SFML 3** with custom tile textures, animated agent sprites, and a catalog of 60 authentic scrap items with rarity tiers.
- **Dynamic Camera Controller:** Smooth dragging with Left Mouse Button (<kbd>LMB</kbd>), smooth zoom ($0.2\times$ to $4.5\times$), keyboard arrow panning, and toggleable auto-follow camera.
- **Tactical Radar Monitor & Hardware Bezel Controls:**
  - Scalable CRT tactical radar overlay in the corner with interactive hardware bezel controls.
  - Power button with CRT screen power-off/on animation and dark-masked LED diode.
  - Dual-state tactical layers toggle switch with smooth animated upper (ON) and lower (OFF) green LED indicators.
  - Interactive zoom slider knob with mouse dragging, camera zoom synchronization, and generous grab padding.
  - Overview rotary dial button with circular cyan hover to instantly fit the entire maze into view.
- **Direct Tactical Manual Piloting (WASD + Diagonal):**
  - Full 8-directional manual agent control with chord detection, deliberate tactical pacing, and accurate energy deduction.
  - Emergency evacuation allowance: allows stepping onto the exit cell from the penultimate tile even under low energy.
  - Instant return to route via <kbd>Z</kbd> hotkey or tactical return HUD card.
- **Interactive Guided Beginner Tour ("Інструкція для чайників"):**
  - Step-by-step interactive tutorial with glowing spotlight cutouts, auto-scrolling sidebar highlights, and safe hold-<kbd>Esc</kbd> exit.
- **Session State Persistence:**
  - Preserves simulation settings, collapsing headers, radar power, camera view, and borderless fullscreen state (<kbd>F11</kbd>) across game restarts.
- **Full Simulation Controls & Safety Warnings:**
  - Real-time speed slider ($0.5\times$ to $4.0\times$), step-by-step frame advance, history stepping backward, instant finish, and reset.
  - Intelligent monitoring of remaining agent energy relative to maze winding factor and distance to extraction point.
- **Batch Benchmarking Suite:** Automated execution comparing all algorithm combinations ($2 \times 3 = 6$ solver configurations) with CSV metric export.

---

## ⌨️ User Controls & Hotkeys Reference

| Key / Action | Function | Description |
| :---: | :--- | :--- |
| <kbd>Space</kbd> | **Play / Pause / Step** | Toggle real-time playback; when paused, tap to advance 1 step or hold for continuous step. |
| <kbd>Ctrl</kbd> + <kbd>Space</kbd> | **Step Backward** | Revert simulation state and energy consumption to the previous historical step. |
| <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd>, <kbd>D</kbd> | **Manual Movement** | Freely pilot the worker agent in 8 directions (orthogonal + diagonal chords). |
| <kbd>Z</kbd> | **Return to Plan** | Instantly return worker agent from manual exploration back to the planned route. |
| <kbd>B</kbd> | **Mission Telemetry** | Toggle the collapsible HUD mission metrics and scrap quota telemetry card. |
| <kbd>M</kbd> | **Tactical Layers** | Toggle tactical overlay layers (Grid lines, Optimal route, Walked cell footprint). |
| <kbd>C</kbd> | **Camera Follow** | Toggle automatic camera tracking on the active worker agent. |
| <kbd>F11</kbd> | **Fullscreen** | Toggle borderless fullscreen display mode. |
| <kbd>Esc</kbd> | **Menu / Tour Abort** | Toggle sidebar control panel; hold during the Beginner Tour to safely abort. |
| <kbd>LMB</kbd> Drag (Map) | **Pan Viewport** | Click and drag with left mouse button to pan camera smoothly across the labyrinth. |
| <kbd>LMB</kbd> (Radar / Bezel) | **Radar Controls** | Click radar to center camera; drag zoom slider, toggle power, or click overview dial. |
| <kbd>Mouse Wheel</kbd> | **Zoom** | Smoothly zoom viewport in and out ($0.2\times$ to $4.5\times$). |

---

## ⚙️ Technical Stack & System Requirements

### Software Stack

- **Language:** C++20 (Standard ISO/IEC 14882:2020)
- **Multimedia & Windowing:** [SFML 3.0.2](https://www.sfml-dev.org/) (Simple and Fast Multimedia Library)
- **Graphical Interface (GUI):** [Dear ImGui](https://github.com/ocornut/imgui) (v1.91+) & [ImGui-SFML](https://github.com/SFML/imgui-sfml)
- **Serialization / Data:** [nlohmann/json](https://github.com/nlohmann/json)
- **Build System:** CMake 3.22+
- **Compiler:** GCC 11+ (MinGW-w64), Clang 13+, or MSVC 2022+

### Minimum System Requirements

- **Operating System:** Windows 10/11 (64-bit), Linux (x86-64), or macOS
- **CPU:** Dual-core x86-64 @ 2.0 GHz or higher
- **RAM:** 4 GB RAM (Application uses $< 70$ MB)
- **GPU:** OpenGL 3.0 / DirectX 11 compatible graphics card
- **Storage:** 50 MB free disk space

---

## 📁 Directory Structure

```text
course-projects/DSA-cpp/
├── CMakeLists.txt                         # CMake build script
├── README.md                              # 📖 Project Documentation (Bilingual: EN / UA)
├── docs/                                  # 📑 Academic documentation & specifications
│   ├── DSA-Sych-Yaroslav-2PI25b.docx      # Final coursework explanatory note manuscript
│   ├── DSA-Sych-Yaroslav-2PI25b.pdf       # Compiled PDF version (DSTU 3008:2015)
│   ├── guidelines-for-coursework-UA.pdf   # Methodological Guidelines (Ukrainian)
│   └── assets/                            # Task variants & figures (3.1.png, 3.2.png)
├── assets/                                # 🎨 Application visual assets & icons
│   ├── app.rc                             # Windows resource definition script
│   ├── icon.ico                           # Executable icon
│   ├── icon.png                           # High-res application icon
│   ├── map.png                            # Floor texture background & CRT monitor frame
│   ├── slider_knob.png                    # Hardware CRT zoom slider knob sprite
│   └── cursor-grabbing*.png               # Camera drag cursors
├── include/                               # 📦 C++ Header-only architecture
│   ├── algorithms/                        # Core algorithmic solvers
│   │   ├── knapsack/                      # Knapsack strategies
│   │   │   ├── IKnapsackSolver.hpp        # Knapsack solver interface
│   │   │   ├── DPKnapsack.hpp             # 0/1 Dynamic Programming solver
│   │   │   └── GreedyKnapsack.hpp         # Value-density heuristic solver
│   │   ├── pathfinding/                   # Grid pathfinding strategies
│   │   │   ├── IPathfinder.hpp            # Pathfinder interface
│   │   │   ├── AStarFinder.hpp            # A* with Octile metric & closedSet
│   │   │   ├── DijkstraFinder.hpp         # Uniform-cost Dijkstra search
│   │   └── routing/                       # Global route optimization
│   │       ├── RoutePlan.hpp              # Route plan data structure & metrics
│   │       └── TSPSolver.hpp              # TSP Nearest Neighbor + 2-Opt solver
│   ├── core/                              # Fundamental data structures
│   │   ├── Backpack.hpp                   # Agent inventory & capacity manager
│   │   ├── Config.hpp                     # Simulation configuration & limits
│   │   ├── Item.hpp                       # Scrap template catalog & item model
│   │   └── Types.hpp                      # Points, directions, coordinates, enums
│   ├── io/                                # Input / Output utilities
│   │   └── CsvExporter.hpp                # Benchmark CSV exporter
│   ├── map/                               # Procedural labyrinth model
│   │   ├── BSPGenerator.hpp               # Binary Space Partitioning generator
│   │   ├── Cell.hpp                       # Tile model (Floor, Wall, Door, Mud)
│   │   ├── Maze.hpp                       # Grid container, neighbors, path cost
│   │   └── Room.hpp                       # BSP room bounding box geometry
│   ├── simulation/                        # Runtime simulation engine
│   │   ├── Agent.hpp                      # Autonomous agent state, energy, steps
│   │   └── SimulationEngine.hpp           # State machine, history stepping, safety
│   └── ui/                                # User Interface & Rendering
│       ├── CameraController.hpp           # Pan & zoom viewport controller
│       ├── ImGuiOverlay.hpp               # Dear ImGui HUD & control panels
│       ├── Minimap.hpp                    # Real-time radar minimap component
│       └── ProceduralRenderer.hpp         # SFML tile & sprite grid renderer
└── src/                                   # 💻 C++ Implementation source files
    ├── main.cpp                           # Application entry point & game loop
    └── map/
        └── BSPGenerator.cpp               # BSP generation implementation
```

---

## 🚀 Building & Running the Project

### 1. Prerequisites

Ensure you have CMake (3.22+) and a modern C++20 compiler installed (MinGW-w64 GCC 13+ recommended for Windows). SFML 3.0.2 should be present on your system (e.g. at `E:/Dev/Dependencies/SFML-3.0.2` or specified via `-DCMAKE_PREFIX_PATH`).

### 2. Configure & Build via CLI

```bash
# Clone or navigate to the repository
cd course-projects/DSA-cpp

# Create and enter build directory
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Compile the target executable
cmake --build build --config Release -j4
```

### 3. Run the Application

```bash
# Launch the compiled executable
./build/LethalCompanyDSA.exe
```

All required SFML runtime DLLs and asset files are automatically copied into `build/` during the CMake post-build step.

---

<br />

---

<a id="-українська-версія"></a>

# 🇺🇦 Українська версія

> 🌐 **Мова:** Українська &nbsp;|&nbsp; **[🇬🇧 Switch to English Version / Перейти до англійської версії](#-english-version)**

> [!IMPORTANT]
> **Тип README-файлу:** 🎮 **Документація до програми / комп'ютерної гри-симулятора**  
> Цей файл присвячено безпосередньо програмному застосунку-симулятору **Lethal Company DSA Simulator**: пайплайну комбінаторних алгоритмів (0/1 рюкзак, TSP з 2-Opt, пошук шляху A\*/Дейкстра/BFS, процедурна генерація BSP), інтерфейсу Dear ImGui, інтерактивному керуванню, C++20 / SFML 3 архітектурі та інструкціям зі збірки проєкту.
>
> 📑 **Шукаєте пояснювальну записку та академічну документацію до курсової роботи?**  
> Перейдіть до окремого **[README пояснювальної записки та документації курсової (`docs/README.md`)](./docs/README.md)**, де представлено структуру пояснювальної записки, затверджене індивідуальне завдання, формальну математичну постановку задачі, вимоги кафедри та нотатки дослідження алгоритмів.

---

## 📑 Документація курсового проєкту та методичні матеріали

| Документ                       | Формат | Опис та пряме посилання                                                                                                                                     |
| :----------------------------- | :----: | :---------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **README курсової записки**    |  `MD`  | 📑 [docs/README.md](./docs/README.md) — Повний путівник по пояснювальній записці, математичній моделі та методичним вимогам                                 |
| **Рукопис записки (DOCX)**     | `DOCX` | 🇺🇦 [`DSA-Sych-Yaroslav-2PI25b.docx`](./docs/DSA-Sych-Yaroslav-2PI25b.docx) — Фінальний офіційний рукопис пояснювальної записки (ДСТУ 3008:2015)            |
| **Пояснювальна записка (PDF)** | `PDF`  | 🇺🇦 [`DSA-Sych-Yaroslav-2PI25b.pdf`](./docs/DSA-Sych-Yaroslav-2PI25b.pdf) — Готовий до друку та перевірки PDF-документ курсової роботи                     |
| **Методичні вказівки кафедри** | `PDF`  | 🇺🇦 [`guidelines-for-coursework-UA.pdf`](./docs/guidelines-for-coursework-UA.pdf) — Офіційні методичні вказівки ВНТУ з дисципліни АСД                        |
| **Матеріали та рисунки**       | `DIR`  | 🇺🇦 [`docs/assets/`](./docs/assets/) — Графічні матеріали (рис. 3.1, рис. 3.2) та реєстр варіантів завдань кафедри                                          |

---

## 🎓 Академічний профіль та метадані

- **Заклад вищої освіти:** Вінницький національний технічний університет ([ВНТУ](https://vntu.edu.ua/))
- **Факультет:** Факультет інформаційних технологій та комп'ютерної інженерії (ФІТКІ)
- **Кафедра:** Кафедра програмної інженерії
- **Спеціальність:** F2 «Інженерія програмного забезпечення»
- **Дисципліна:** «Алгоритми та структури даних» (АСД)
- **Виконавець:** студент групи 2ПІ-25б, **Сич Ярослав Олександрович**
- **Керівник проєкту:** к.т.н., доцент кафедри ПЗ Ткаченко О.М.
- **Рік та місце виконання:** Вінниця, 2026

---

## 🎯 Тема, мета та завдання курсового проєкту

### Тема роботи

> **«Розробка та дослідження ефективності алгоритмів дискретної оптимізації маршруту та збору ресурсів у сітковому лабіринті (Симулятор Lethal Company DSA)»**
> _(Загальна тема: «Розробка й аналіз ефективності алгоритмів і програм»)_

### Актуальність та концепція гейміфікації

Проєкт моделює роботу автономного агента-експерта у стилістиці популярної гри **Lethal Company**. Співробітник корпорації висаджується в покинутому процедурно згенерованому підземному комплексі для збору цінних ресурсів (металобрухту). Агент має обмежений запас автономної енергії, лімітовану вантажопідйомність рюкзака та повинен виконати корпоративну квоту мінімального прибутку, повернувшись до точки шлюзу.

Розв'язання поставленої задачі вимагає дворівневого комбінаторного оптимізаційного пайплайну:

1. **Оптимізація вибору ресурсів:** Розв'язання задачі про рюкзак (Knapsack Problem) для максимізації вартості вантажу в межах граничної ваги.
2. **Оптимізація черговості обходу:** Розв'язання задачі комівояжера (TSP) із фіксованими точками старту та евакуації для мінімізації загальної довжини туру.
3. **Низькорівнева навігація в сітці:** Пошук енергетично найвигідніших траєкторій без колізій на 8-зв'язному зваженому графі лабіринту з урахуванням стін, дверей та багнистих ділянок.

### Основні завдання проєкту

1. Розробити формальну математичну модель комплексної оптимізаційної задачі.
2. Реалізувати та порівняти альтернативні алгоритмічні підходи:
   - **Задача про рюкзак:** Точний алгоритм динамічного програмування ($O(N \cdot C)$) проти наближеного жадібного алгоритму за питомою цінністю ($O(N \log N)$).
   - **Пошук шляху на графах:** Евристичний алгоритм A\* (з октагональною метрикою) проти алгоритму Дейкстри (рівномірний пошук за вартістю) та неінформованого пошуку в ширину (BFS).
   - **Маршрутизація TSP:** Метод найближчого сусіда (Nearest Neighbor) із локальною оптимізацією 2-Opt.
3. Реалізувати процедурну генерацію лабіринту алгоритмом **BSP (Binary Space Partitioning)** із гарантованою досяжністю кімнат, циклічними коридорами та генерацією ділянок болота.
4. Створити графічний інтерфейс користувача мовою **C++20** на базі бібліотек **SFML 3** (апаратний рендеринг) та **Dear ImGui** (аналітичний HUD та керування).
5. Забезпечити вимірювання ключових характеристик: часу виконання ($\mu s$), пам'яті (байти), кількості розкритих вузлів, довжини шляху, вартості зібраного вантажу та витраченої енергії.
6. Реалізувати модуль автоматичного бенчмаркінгу з експортом метрик у CSV.

---

## 🧩 Алгоритмічна архітектура та структури даних

```mermaid
flowchart TD
    A[Старт експедиції / Новий Seed] --> B[Генератор лабіринту BSP]
    B --> C[Спавн предметів брухту та Шлюзу]
    C --> D{Оптимізатор рюкзака}
    D -->|Точний| D1[0/1 DP Knapsack]
    D -->|Наближений| D2[Жадібний Knapsack]
    D1 --> E[Обраний піднабір цінних предметів]
    D2 --> E
    E --> F[Планувальник маршруту TSP]
    F --> F1[Метод найближчого сусіда]
    F1 --> F2[Локальна оптимізація 2-Opt]
    F2 --> G[Оптимальна послідовність точок]
    G --> H{Пошук шляху між точками}
    H -->|A* Октагональний| H1[AStarFinder]
    H -->|Дейкстра| H2[DijkstraFinder]
    H -->|BFS у ширину| H3[BFSFinder]
    H1 --> I[Зшитий безаварійний маршрут]
    H2 --> I
    H3 --> I
    I --> J[Симулятор / SFML + ImGui вікно]
```

### 1. Порівняльна характеристика реалізованих алгоритмів

| Етап             | Алгоритм                 |           Складність (Час / Пам'ять)           | Особливості реалізації                                                                                                           |
| :--------------- | :----------------------- | :--------------------------------------------: | :------------------------------------------------------------------------------------------------------------------------------- |
| **Вибір брухту** | **0/1 Knapsack DP**      | $O(N \cdot \min(C, \sum W)) \ /\ O(N \cdot C)$ | Точний глобальний оптимум; безпечне виділення пам'яті та миттєвий результат, якщо всі предмети вміщуються в рюкзак.              |
| **Вибір брухту** | **Greedy Knapsack**      |            $O(N \log N) \ /\ O(N)$             | Швидке сортування за питомою цінністю $V_i / W_i$; пріоритетна обробка предметів нульової ваги через `infinity`.                 |
| **Тур TSP**      | **TSP Nearest Neighbor** |               $O(K^2) \ /\ O(K)$               | Жадібна побудова ланцюжка між $K$ точками з фіксованими точками старту та евакуації; безпечна обробка ізольованих вершин.        |
| **Тур TSP**      | **TSP 2-Opt Search**     |           $O(M \cdot K^2) \ /\ O(K)$           | Локальна оптимізація обміну 2 ребер; захист від нескінченної вартості ребер при частковій недосяжності.                          |
| **Навігація**    | **A\* Pathfinding**      |            $O(E \log V) \ /\ O(V)$             | 8-напрямковий пошук з октагональною евристикою $h = 10(dx+dy) - 6\min(dx,dy)$; `closedSet` усуває дублікати та застарілі записи. |
| **Навігація**    | **Алгоритм Дейкстри**    |            $O(E \log V) \ /\ O(V)$             | Рівномірний пошук за мінімальною вартістю енергії; еталон точності вартості переходів.                                           |
| **Навігація**    | **Пошук у ширину (BFS)** |              $O(V + E) \ /\ O(V)$              | Неінформований хвильовий пошук; знаходить шлях із найменшою кількістю кроків без врахування вартості покриття.                   |
| **Топологія**    | **Генератор BSP**        |        $O(R \log R) \ /\ O(W \cdot H)$         | Двійкове розбиття простору з вирубкою кімнат, сполучних коридорів та додаткових циклічних петель.                                |

### 2. Використані базові структури даних

| Абстрактна структура даних  | Контейнер у C++20                                        | Застосування в проєкті                                                                          |
| :-------------------------- | :------------------------------------------------------- | :---------------------------------------------------------------------------------------------- |
| **Динамічний 2D масив**     | `std::vector<std::vector<Cell>>`                         | Матриця комірок лабіринту (стіни, підлога, шлюз, вихід, болото).                                |
| **Бінарна купа (Min-Heap)** | `std::priority_queue<Node, vector<Node>, greater<Node>>` | Черга з пріоритетом для швидкого отримання вузла з найменшим $f$ або $g$ у A\* та Дейкстрі.     |
| **Черга FIFO**              | `std::queue<core::Point>`                                | Черга для покрокового хвильового обходу графа у BFS.                                            |
| **Хеш-таблиці / Множини**   | `std::unordered_map`, `std::unordered_set`               | Збереження відстаней `gScore`, батьківських вузлів `cameFrom` та відвіданих клітинок за $O(1)$. |
| **Матриця значень DP**      | `std::vector<std::vector<int>>`                          | Таблиця динамічного програмування для розв'язання задачі про рюкзак.                            |
| **Стек / Вектор предків**   | `std::vector<core::Point>` із `std::reverse`             | Відновлення знайденого ланцюжка кроків від фінішу до старту.                                    |

---

## 🎮 Інтерактивна симуляція та графічний інтерфейс

- **Процедурний графічний рушій:** Апаратне прискорення через **SFML 3**, текстурована сітка кімнат, анімований спрайт агента та розширений каталог із 60 автентичних предметів брухту Lethal Company з градацією рідкості.
- **Динамічна камера:** Плавне переміщення лівою кнопкою миші (<kbd>ЛКМ</kbd>), плавний зум ($0.2\times$ – $4.5\times$), переміщення стрілками клавіатури та режим авто-слідкування за персонажем.
- **Тактичний ЕПТ-радар та інтерактивні апаратні органи керування на рамці:**
  - Масштабована міні-карта у вигляді тактичного ЕПТ-монітора з анімованим індикатором масштабу.
  - Кнопка живлення з плавною анімацією ввімкнення/вимкнення ЕПТ-екрана та маскуванням діода у вимкненому стані.
  - Тумблер перемикання тактичних шарів (сітка, маршрут, слід) із плавним перехресним засвіченням верхнього (ON) та нижнього (OFF) зелених світлодіодів.
  - Апаратний інтерактивний повзунок зуму з плавним перетягуванням мишкою, синхронізацією з камерою та розширеною буферною зоною наведення курсора.
  - Кругла кнопка огляду комплексу з неоновим блакитним підсвічуванням для миттєвої підгонки масштабу під увесь лабіринт.
- **Пряме тактичне пілотування робітником (WASD + діагоналі):**
  - Повноцінний 8-напрямковий рух клавішами з акордовою обробкою діагоналей, виваженим тактичним темпом та динамічним списанням ходової енергії.
  - Аварійна дотяжка до евакуації: можливість перейти на клітинку виходу з передостанньої позиції навіть за нестачі енергії на стандартний крок.
  - Миттєве повернення до алгоритмічного плану експедиції клавішею <kbd>Z</kbd> або через тактичну картку на екрані.
- **Інтерактивна покрокова інструкція («Інструкція для чайників»):**
  - Інтерактивний 17-кроковий туторіал для новачків із підсвічуванням елементів неоновим прожектором, автопрокруткою сайдбару та безпечним утриманням <kbd>Esc</kbd> для виходу.
- **Збереження стану сесії між перезапусками:**
  - Повне збереження положення камери, стану згортання панелей, живлення термінала та повноекранного режиму (<kbd>F11</kbd>).
- **Повний контроль симуляції та система безпеки:**
  - Слайдер швидкості ($0.5\times$ – $4.0\times$), покроковий хід уперед/назад, миттєве завершення та скидання до старту.
  - Розрахунок мінімальної енергії до точки евакуації з урахуванням звивистості коридорів лабіринту та завчасний перехід у стан `Evacuating`.
- **Модуль пакетного тестування (Batch Benchmark):** Одночасний запуск усіх 6 комбінацій алгоритмів із побудовою підсумкової порівняльної таблиці та експортом у CSV.

---

## ⌨️ Керування та гарячі клавіші

| Клавіша / Дія | Призначення | Опис дії |
| :---: | :--- | :--- |
| <kbd>Пробіл</kbd> | **Старт / Пауза / Крок** | Зупинити або продовжити рух; на паузі: один клік — 1 крок, утримання — безперервний хід. |
| <kbd>Ctrl</kbd> + <kbd>Пробіл</kbd> | **Крок назад** | Відкотити стан симуляції та показники енергії на один крок назад за історією. |
| <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd>, <kbd>D</kbd> | **Ручне керування** | Пряме 8-напрямкове пілотування робітником комплексом (ортогонально та по діагоналях). |
| <kbd>Z</kbd> | **Повернення до плану** | Миттєво повернути робітника з ручного дослідження до останньої планової точки маршруту. |
| <kbd>B</kbd> | **Показники місії** | Відкрити/згорнути плаваючу HUD-картку квоти брухту та телеметрії місії. |
| <kbd>M</kbd> | **Тактичні шари** | Увімкнути/вимкнути тактичні шари (координатну сітку, лінію маршруту, слід кроків). |
| <kbd>C</kbd> | **Автокамера** | Увімкнути/вимкнути динамічне фокусування камери на агенті під час його руху. |
| <kbd>F11</kbd> | **Повноекранний режим** | Перемкнути режим безрамкового повноекранного відображення (Borderless Fullscreen). |
| <kbd>Esc</kbd> | **Меню / Вихід з туру** | Відкрити/закрити бічну панель керування; під час інтерактивного туру — утримання для виходу. |
| <kbd>ЛКМ</kbd> перетягування (Карта) | **Панорамування** | Затиснути ліву кнопку миші для плавного переміщення камери лабіринтом. |
| <kbd>ЛКМ</kbd> (Радар / Рамка) | **Керування радаром** | Центрування камери по кліку, перетягування повзунка зуму, тумблер шарів, кнопка живлення та огляду. |
| <kbd>Колесо миші</kbd> | **Зум** | Плавне наближення та віддалення камери ($0.2\times$ – $4.5\times$). |

---

## ⚙️ Системні вимоги та стек технологій

### Програмний стек

- **Мова програмування:** C++20 (стандарт ISO/IEC 14882:2020)
- **Графічний рушій та вікно:** [SFML 3.0.2](https://www.sfml-dev.org/) (Simple and Fast Multimedia Library)
- **Графічний інтерфейс (GUI):** [Dear ImGui](https://github.com/ocornut/imgui) (v1.91+) та [ImGui-SFML](https://github.com/SFML/imgui-sfml)
- **Обробка даних:** [nlohmann/json](https://github.com/nlohmann/json)
- **Система збірки:** CMake 3.22+
- **Компілятор:** GCC 11+ (MinGW-w64), Clang 13+, або MSVC 2022+

### Апаратні вимоги

- **Операційна система:** Windows 10/11 (64-bit), Linux (x86-64), macOS
- **Процесор:** 2-ядерний процесор з частотою від 2.0 ГГц
- **Оперативна пам'ять:** 4 ГБ RAM (застосунок споживає $< 70$ МБ)
- **Відеокарта:** Адаптер з підтримкою OpenGL 3.0 / DirectX 11
- **Дисковий простір:** 50 МБ вільного місця

---

## 📁 Структура каталогу

```text
course-projects/DSA-cpp/
├── CMakeLists.txt                         # Конфігурація збірки CMake
├── README.md                              # 📖 Документація проєкту (Bilingual: EN / UA)
├── docs/                                  # 📑 Академічна документація та специфікації
│   ├── DSA-Sych-Yaroslav-2PI25b.docx      # Фінальний рукопис пояснювальної записки
│   ├── DSA-Sych-Yaroslav-2PI25b.pdf       # Скомпільований PDF-файл (ДСТУ 3008:2015)
│   ├── guidelines-for-coursework-UA.pdf   # Методичні вказівки кафедри (Українська)
│   └── assets/                            # Варіанти завдань та рисунки (3.1.png, 3.2.png)
├── assets/                                # 🎨 Графічні ресурси, текстури та іконка
│   ├── app.rc                             # Скрипт ресурсів програми (Windows)
│   ├── icon.ico                           # Іконка виконуваного файлу
│   ├── icon.png                           # Графічна іконка вікна
│   ├── map.png                            # Текстура фону комплексу та рамка ЕПТ-радара
│   ├── slider_knob.png                    # Спрайт апаратного повзунка масштабу
│   └── cursor-grabbing*.png               # Спрайти курсорів перетягування
├── include/                               # 📦 Заголовні файли C++ (Header-only архітектура)
│   ├── algorithms/                        # Реалізації алгоритмів
│   │   ├── knapsack/                      # Алгоритми задачі про рюкзак
│   │   │   ├── IKnapsackSolver.hpp        # Інтерфейс оптимізатора рюкзака
│   │   │   ├── DPKnapsack.hpp             # Точний алгоритм динамічного програмування
│   │   │   └── GreedyKnapsack.hpp         # Наближений жадібний алгоритм за щільністю
│   │   ├── pathfinding/                   # Алгоритми пошуку найкоротшого шляху
│   │   │   ├── IPathfinder.hpp            # Інтерфейс алгоритмів навігації
│   │   │   ├── AStarFinder.hpp            # A* з октагональною евристикою та closedSet
│   │   │   ├── DijkstraFinder.hpp         # Алгоритм Дейкстри
│   │   │   └── BFSFinder.hpp              # Неінформований пошук у ширину (BFS)
│   │   └── routing/                       # Глобальне планування маршруту
│   │       ├── RoutePlan.hpp              # Структури даних плану маршруту та метрик
│   │       └── TSPSolver.hpp              # Розв'язувач TSP (Nearest Neighbor + 2-Opt)
│   ├── core/                              # Базові структури та типи
│   │   ├── Backpack.hpp                   # Модель рюкзака та перевірка місткості
│   │   ├── Config.hpp                     # Налаштування розмірів, енергії та квот
│   │   ├── Item.hpp                       # Каталог із 60 предметів брухту
│   │   └── Types.hpp                      # Координати Point, напрямки, переліки
│   ├── io/                                # Модулі вводу/виводу
│   │   └── CsvExporter.hpp                # Експортер результатів бенчмарків у CSV
│   ├── map/                               # Модель лабіринту
│   │   ├── BSPGenerator.hpp               # Генератор лабіринту методом BSP
│   │   ├── Cell.hpp                       # Модель клітинки (Підлога, Стіна, Двері, Болото)
│   │   ├── Maze.hpp                       # Контейнер сітки, перевірка прохідності
│   │   └── Room.hpp                       # Геометрія кімнат BSP
│   ├── simulation/                        # Рушій симуляції
│   │   ├── Agent.hpp                      # Стан, кроки та енергія робітника
│   │   └── SimulationEngine.hpp           # Машина станів, покроковий відкат та безпека
│   └── ui/                                # Підсистема графічного інтерфейсу
│       ├── CameraController.hpp           # Керування камерою (панорамування, зум)
│       ├── ImGuiOverlay.hpp               # Панелі налаштувань та HUD на Dear ImGui
│       ├── Minimap.hpp                    # Компонент динамічної міні-карти
│       └── ProceduralRenderer.hpp         # Процедурний рендерер сітки та спрайтів
└── src/                                   # 💻 Вихідний код реалізації
    ├── main.cpp                           # Головна точка входу та ігровий цикл
    └── map/
        └── BSPGenerator.cpp               # Реалізація алгоритму BSP генерації
```

---

## 🚀 Збірка та запуск проєкту

### 1. Попередні вимоги

Переконайтеся, що у системі встановлено CMake (версії 3.22+) та сучасний компілятор із підтримкою стандарту C++20 (наприклад, MinGW-w64 GCC 13+ або MSVC 2022). Також має бути встановлена бібліотека SFML 3.0.2 (наприклад, за шляхом `E:/Dev/Dependencies/SFML-3.0.2` або через передачу `-DCMAKE_PREFIX_PATH`).

### 2. Конфігурація та збірка через командний рядок

```bash
# Перейти до каталогу курсового проєкту
cd course-projects/DSA-cpp

# Генерація файлів збірки CMake у режимі Release
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Компіляція виконуваного файлу
cmake --build build --config Release -j4
```

### 3. Запуск застосунку

```bash
# Запуск скомпільованого бінарника
./build/LethalCompanyDSA.exe
```

Усі необхідні динамичні бібліотеки (`*.dll`) SFML та графічні ресурси автоматично копіюються в папку `build/` на етапі постобробки CMake (`POST_BUILD`).

