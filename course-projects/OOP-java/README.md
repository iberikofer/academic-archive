<a id="-english-version"></a>

# 🇬🇧 English Version

> 🌐 **Language:** English &nbsp;|&nbsp; **[🇺🇦 Switch to Ukrainian Version / Перейти до української версії](#-українська-версія)**

---

> [!NOTE]
> **Complete Academic & Software Archive / Повноцінний академічний та програмний архів**  
> This folder contains both the **interactive desktop game-simulation application** (**TORNADO GYM Simulator**) developed in Java 17+ and JavaFX, and the comprehensive **academic coursework documentation** (49-page explanatory note, UML models, and university guidelines in `docs/`).

---

## 📑 Project Deliverables & Documentation

| Deliverable / Document | Format | Description & Link |
| :------------------------------ | :----: | :--------------------------------------------------------------------------------------------------------------------------- |
| **Interactive Game Application** | `JavaFX` | 🎮 Complete desktop application source code in [`src/main/java/gymworld/`](./src/main/java/gymworld/) (launch via `./mvnw clean javafx:run`) |
| **Coursework Explanatory Note** | `PDF`  | 🇬🇧 [COURSEWORK-EN.pdf](./docs/COURSEWORK-EN.pdf) — Complete 49-page technical report in English |
| **Methodological Guidelines**   | `PDF`  | 🇬🇧 [guidelines-for-coursework-EN.pdf](./docs/guidelines-for-coursework-EN.pdf) — Official VNTU coursework requirements in English |
| **Editable Report Document**     | `DOCX` | 📄 [OOP-SychYaroslav-2PI-25B.docx](./docs/OOP-SychYaroslav-2PI-25B.docx) — Full editable thesis manuscript in Microsoft Word |

---

## 🎓 Academic Profile & Metadata

- **Institution:** Vinnytsia National Technical University ([VNTU](https://vntu.edu.ua/))
- **Faculty:** Faculty of Information Technology and Computer Engineering (FITCE)
- **Department:** Department of Software Engineering
- **Program / Major:** F2 Software Engineering
- **Course / Discipline:** Object-Oriented Programming (OOP)
- **Author:** Yaroslav SYCH, Student of Group 2PI-25b
- **Academic Advisor / Supervisor:** Ph.D. in Engineering, Associate Professor Oleksandr RESHETNIK
- **Evaluation / Result:** **82 / 100** (ECTS Grade: **B**)
- **Year & Location:** Vinnytsia, 2025

---

## 🎯 Topic & Project Objectives

### Project Topic

> **"Development of a software system for modeling of objects: «TORNADO GYM Simulator. Macro-objects: Front Desk & Intake, Cardio Area, Weights Area. Micro-objects: Athlete, Power Lifter, Gym Boss» using UML and the Java programming language"**

### Background & Social Significance

According to World Health Organization (WHO) data, over **1.4 billion adults worldwide** lead a sedentary lifestyle, increasing the risk of cardiovascular diseases and mental fatigue. The **Tornado GYM Simulator** addresses this issue by leveraging gamification and positive reinforcement (energy recovery, muscle hypertrophy mechanics, achievement feedback) to motivate users toward fitness and healthy lifestyle habits.

### Main Tasks

1. Analyze modern trends in gamification and interactive simulation systems.
2. Formulate system architecture and formal Technical Specifications (TS).
3. Design comprehensive **UML models** (Inheritance, Aggregation, Composition, Association, Cooperation, Sequence, and State Machine diagrams).
4. Implement a clean, modular desktop application using **Java 17+** and **JavaFX**.
5. Implement micro-object dynamic behaviors, multi-level inheritance, collision detection with macro-zones, state persistence (`.gym` serialization), and analytical Stream API filtering queries.
6. Conduct verification, stress testing, and author user documentation.

---

## 🚀 How to Run the Game Application

### Prerequisites

- **Java Development Kit (JDK):** Version 17 or higher (tested with JDK 21).
- **Maven:** Optional (the repository includes the Maven Wrapper `mvnw` / `mvnw.cmd`, so no pre-installed Maven is required).

### Launching via Terminal (Maven Wrapper)

Open a terminal in this project directory (`course-projects/OOP-java`) and execute:

```bash
# On Linux / macOS:
./mvnw clean javafx:run

# On Windows (PowerShell or Command Prompt):
.\mvnw.cmd clean javafx:run
```

### Launching via IDE (IntelliJ IDEA / VS Code / Eclipse)

1. Open the `course-projects/OOP-java` folder in your IDE as a Maven project.
2. Allow the IDE to import project dependencies from [`pom.xml`](./pom.xml).
3. Run the `main()` method in [`src/main/java/gymworld/Launcher.java`](./src/main/java/gymworld/Launcher.java) or [`src/main/java/gymworld/GymWorld.java`](./src/main/java/gymworld/GymWorld.java).

---

## 💻 Software Architecture & Codebase Structure

The software is organized into a clean domain model inside the package `gymworld` ([`src/main/java/gymworld/`](./src/main/java/gymworld/)):

| Component / Class | Role & Implementation Details |
| :--- | :--- |
| [`Launcher.java`](./src/main/java/gymworld/Launcher.java) | Bootstrap entry point that delegates to `GymWorld.class`, bypassing JavaFX runtime module restrictions. |
| [`GymWorld.java`](./src/main/java/gymworld/GymWorld.java) | Central game engine window. Implements the 60 FPS `AnimationTimer` game loop, canvas viewport rendering, keyboard/mouse input dispatching, collision detection, and camera scrolling. |
| [`Athlete.java`](./src/main/java/gymworld/Athlete.java) | Base micro-object class representing gym visitors. Encapsulates stamina mechanics (1–100), stamina decay, dynamic energy bar UI, zone detection, and deep cloning (`Cloneable`). |
| [`PowerLifter.java`](./src/main/java/gymworld/PowerLifter.java) | Derived athlete class specialized in heavy strength workouts. Overrides `train()` with higher power output and stamina burn; tracks maximum squat, bench press, and deadlift. |
| [`GymBoss.java`](./src/main/java/gymworld/GymBoss.java) | Elite gym authority character extending `PowerLifter`. Features the polymorphic `motivate(List<Athlete>)` aura that restores stamina and buffs nearby athletes. |
| [`TrainingZone.java`](./src/main/java/gymworld/TrainingZone.java) | Macro-objects (Front Desk, Cardio Area, Weights Area). Handles coordinate boundary checks (`intersects`), dynamic visitor counters, and visual area representation. |
| [`Equipment.java`](./src/main/java/gymworld/Equipment.java) | Domain model representing fitness gear (dumbbells, barbells, mats) used by athletes. |
| [`Minimap.java`](./src/main/java/gymworld/Minimap.java) | Real-time radar HUD minimap located in the upper-left corner. Displays scaled positions of all zones and athletes; clicking teleports the main camera viewport. |
| [`SerializationManager.java`](./src/main/java/gymworld/SerializationManager.java) | Persistent binary state engine. Saves and restores world snapshots to binary `.gym` packages via `ObjectOutputStream` and `ObjectInputStream` using safe `try-with-resources`. |
| [`InsertDialog.java`](./src/main/java/gymworld/InsertDialog.java) | JavaFX modal dialog (<kbd>Insert</kbd>) for dynamically instantiating new athletes with custom parameters. |
| [`EditDialog.java`](./src/main/java/gymworld/EditDialog.java) | Contextual modal dialog (<kbd>Right Click</kbd>) for modifying attributes of existing athletes. |
| [`SearchDialog.java`](./src/main/java/gymworld/SearchDialog.java) | Modal query window for searching micro-objects by specific domain criteria. |
| [`FilterDialog.java`](./src/main/java/gymworld/FilterDialog.java) | Advanced analytics table (<kbd>G</kbd>) with multi-criteria sorting (Name, Weight, Stamina) and filtering powered by the Java Stream API. |

### Graphical Resources & Styling

Located in [`src/main/resources/gymworld/`](./src/main/resources/gymworld/):
- **Textures & Sprites:** `athlete.png`, `power_lifter.png`, `gym_boss.png`, `front_desk.png`, `cardio_area.png`, `weights_area.png`, `bicep.png`, `lightning.png`, `wave.png`.
- **Stylesheet:** `style.css` (custom dark theme, HUD styling, modal dialog inputs).

---

## 🏗️ Domain Architecture & Model Hierarchy

The simulation takes place within a 2D fitness center environment (`GymWorld`) featuring interactive micro-objects (gym visitors) and macro-objects (training zones).

### 1. Micro-Objects (3-Level Inheritance Hierarchy)

- **[`Athlete`](./docs/assets/athlete-trimmed.png) (Base Class):**
  - Represents a regular gym visitor (black Tornado GYM t-shirt & shorts).
  - **Fields:** `name`, `weight`, `energyLvl` (1–100), `equipment`, `x`, `y`, `isActive`, `currentZone`.
  - **Visuals:** Sprite texture (`ImageView`), status text block, dynamic yellow stamina bar (`energyBar`), activation border (`activeBorder`), zone status indicator.
  - **Behaviors:** Standard navigation, stamina decay during workouts, energy restoration via `rest()` / `eat()`, deep cloning via `Cloneable`.
- **[`Power Lifter`](./docs/assets/athlete-power-lifter-trimmed.png) (Extends `Athlete`):**
  - Experienced heavy strength athlete (blue weightlifting suit).
  - **Additional Fields:** `maxBenchPress`, `maxDeadlift`, `maxSquat`.
  - **Behaviors:** Overrides `train()` with higher stamina consumption and rapid strength scaling, specialized for the Weights Area.
- **[`Gym Boss`](./docs/assets/athlete-gym-boss-trimmed.png) (Extends `Power Lifter`):**
  - Elite authority figure of the gym (red tracksuit).
  - **Additional Fields:** `authorityLevel`, `gymMastery`.
  - **Behaviors:** Unique `motivate(List<Athlete>)` method demonstrating dynamic polymorphism (increases stamina recovery and performance of nearby athletes).

### 2. Macro-Objects (Training Zones)

- **[`Front Desk & Intake`](./docs/assets/front-desk-and-intake-graph.png) (600 × 280 px):** Reception desk with illuminated Tornado logo, protein/supplement displays, visitor check-in.
- **[`Cardio Area`](./docs/assets/cardio-area-graph.png) (480 × 320 px):** Treadmills, stationary bikes, and ellipticals.
- **[`Weights Area`](./docs/assets/weights-area-graph.png) (480 × 320 px):** Power racks, Olympic barbells, dumbbells, and adjustable benches.
- **Features:**
  - Coordinate intersection detection via `getBoundsInParent().intersects(...)`.
  - Dynamic counter indicator (`countText`): red when empty (`0`), black when occupied (`>0`).
  - Bidirectional tracking (`addAthlete(a)` and `removeAthlete(a)`).

### 3. Universal Environment (`GymWorld`)

- Main game canvas with real-time `AnimationTimer` game loop.
- **Dynamic Viewport:** Displays a focused area ($\le 25\%$ of total world dimension).
- **Interactive Mini-Map:** Real-time mini-radar in the upper-left corner showing all zones and athlete positions. Clicking the mini-map instantly moves the main viewport.

---

## 🧩 Key OOP Concepts & Design Patterns

| OOP Concept              | Implementation in Tornado GYM                                                                                                                                                                  |
| :----------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Abstraction**          | Abstract base models and clear separation between business logic and JavaFX graphical rendering nodes.                                                                                         |
| **Encapsulation**        | Strict `private`/`protected` access modifiers, validation in getters/setters, state integrity preservation.                                                                                    |
| **Inheritance**          | 3-tier hierarchy: `Athlete` $\rightarrow$ `PowerLifter` $\rightarrow$ `GymBoss`. Proper `super()` constructor chaining.                                                                        |
| **Polymorphism**         | **Static:** Overloaded methods/queries. **Dynamic:** Overridden `train()` and `motivate()` methods; `instanceof` type checking.                                                                |
| **Composition**          | `TrainingZone` owns its graphical primitives (`ImageView`, `Text`, `Rectangle`, `Group`), destroyed alongside the zone.                                                                        |
| **Aggregation**          | `GymWorld` manages collections of `Athlete` and `TrainingZone` instances.                                                                                                                      |
| **Association**          | Bidirectional relation: `Athlete.currentZone` $\leftrightarrow$ `TrainingZone.zoneAthletes`.                                                                                                   |
| **Deep Cloning**         | Implementation of `Cloneable` with independent memory cloning (`Ctrl+C`).                                                                                                                      |
| **Object Serialization** | Binary `.gym` state persistence via `Serializable` and `ObjectOutputStream` / `ObjectInputStream`. Non-serializable JavaFX nodes are marked `transient` and reconstructed via `initVisuals()`. |
| **Stream API & Lambdas** | Declarative data filtering, sorting criteria (Name, Weight, Stamina), and analytical queries.                                                                                                  |

---

## 📊 UML Modeling & System Diagrams

The project is thoroughly designed and verified against the following UML diagrams (located in [`docs/assets/`](./docs/assets/)):

1. **Class Inheritance Diagram** ([`.UML-diagram-3.1.png`](./docs/assets/.UML-diagram-3.1.png)) — 3-level athlete hierarchy with fields and methods.
2. **Aggregation Diagram** ([`.UML-diagram-3.2.png`](./docs/assets/.UML-diagram-3.2.png)) — Container-component relations between `GymWorld`, `TrainingZone`, and `Athlete`.
3. **Composition Diagram** ([`.UML-diagram-3.3.png`](./docs/assets/.UML-diagram-3.3.png)) — Internal graphical node composition inside `TrainingZone`.
4. **Association Diagram** ([`.UML-diagram-3.4.png`](./docs/assets/.UML-diagram-3.4.png)) — Bidirectional links and reference synchronization.
5. **Cooperation (Communication) Diagram** ([`.UML-diagram-3.5.png`](./docs/assets/.UML-diagram-3.5.png)) — Message passing during collision detection.
6. **Sequence Diagram** ([`.UML-diagram-3.6.png`](./docs/assets/.UML-diagram-3.6.png)) — Chronological sequence of zone entry, counter update, and emoji badge trigger.
7. **State Machine Diagram** ([`.UML-diagram-3.7.png`](./docs/assets/.UML-diagram-3.7.png)) — Life cycle transitions: `Idle` $\rightarrow$ `Moving` $\rightarrow$ `Training` $\rightarrow$ `Exhausted` $\rightarrow$ `Resting`.

---

## 🎮 User Controls & Hotkey Reference

|                                               Key / Action                                                | Function                       | Description                                                                                                       |
| :-------------------------------------------------------------------------------------------------------: | :----------------------------- | :---------------------------------------------------------------------------------------------------------------- |
|                                             <kbd>Insert</kbd>                                             | **Add Athlete Dialog**         | Opens GUI modal (`TextField`, `ChoiceBox`, `RadioButton`) to create a new `Athlete`, `PowerLifter`, or `GymBoss`. |
|                                             <kbd>Delete</kbd>                                             | **Remove Athlete**             | Deletes the currently selected athlete from the simulation.                                                       |
|                                      <kbd>Ctrl</kbd> + <kbd>C</kbd>                                       | **Clone Athlete**              | Performs deep cloning of the active character with identical properties offset on the map.                        |
|                                      <kbd>Ctrl</kbd> + <kbd>S</kbd>                                       | **Save State**                 | Opens `FileChooser` dialog to export simulation state to a `.gym` binary file.                                    |
|                                      <kbd>Ctrl</kbd> + <kbd>L</kbd>                                       | **Load State**                 | Opens `FileChooser` dialog to load and restore world state from `.gym` file.                                      |
|                                               <kbd>G</kbd>                                                | **Statistics & Sorting**       | Opens dynamic statistics table with multi-criteria sorting (Name, Weight, Energy Level).                          |
| <kbd>W</kbd> <kbd>A</kbd> <kbd>S</kbd> <kbd>D</kbd> / <kbd>↑</kbd> <kbd>←</kbd> <kbd>↓</kbd> <kbd>→</kbd> | **Move Character**             | Moves the currently selected active athlete across the gym.                                                       |
|                                        <kbd>Left Mouse Click</kbd>                                        | **Select / Mini-Map Teleport** | Click character to select / Click mini-map to instantly move viewport camera.                                     |
|                                       <kbd>Right Mouse Click</kbd>                                        | **Edit Athlete Dialog**        | Opens contextual property editor (weight, stamina, equipment) for the clicked athlete.                            |

### Implemented Analytical Queries

- 🔍 **Query 1:** Find micro-object by user-defined parameters with location and current zone reporting.
- 📋 **Query 2:** List all micro-objects assigned to a specific macro-zone.
- 🚫 **Query 3:** List all unassigned micro-objects (wandering freely outside any zone).
- 🔢 **Counting Queries:** Active athletes count, athletes with stamina $> 50\%$, and athletes located in the right half of the gym.
- 🔀 **Multi-Criteria Sorting:** Sort by Name (A–Z / Z–A), Weight, and Energy Level.

---

## ⚙️ Technical Stack & Hardware Requirements

### Software Stack

- **Language:** Java SE 17+ (LTS)
- **GUI Framework:** OpenJFX (JavaFX) 21+ (Hardware-accelerated rendering)
- **Build System:** Apache Maven (with included Maven Wrapper `mvnw`)
- **IDE:** IntelliJ IDEA / Eclipse / NetBeans / VS Code
- **Architecture:** Object-Oriented Architecture with Model-View separation

### System Requirements

- **OS:** Windows 10/11, macOS, or Linux
- **CPU:** Intel/AMD x86-64 @ 2.2 GHz or higher
- **RAM:** 8 GB DDR4 (Simulation consumes ~270 MB)
- **Storage:** 25 MB free disk space
- **GPU:** OpenGL 3.0+ / DirectX 11 compatible graphics card (2 GB VRAM)

---

## 📁 Directory Structure

```text
course-projects/OOP-java/
├── pom.xml                               # Maven project configuration (Java 17, JavaFX 21, JUnit 5)
├── mvnw / mvnw.cmd                       # Maven Wrapper for cross-platform execution without installed Maven
├── README.md                             # 📖 Project Documentation (Bilingual: EN / UA)
├── src/                                  # 🎮 Application source code and assets
│   └── main/
│       ├── java/gymworld/                # Java source code
│       │   ├── Launcher.java             # Entry point bootstrap
│       │   ├── GymWorld.java             # Main application, game loop & viewport
│       │   ├── Athlete.java              # Base domain entity (visitor)
│       │   ├── PowerLifter.java          # Derived domain entity (strength athlete)
│       │   ├── GymBoss.java              # Derived elite entity (gym master)
│       │   ├── TrainingZone.java         # Macro-object (gym zones & collision handling)
│       │   ├── Equipment.java            # Domain entity for gym equipment
│       │   ├── Minimap.java              # Real-time radar HUD minimap
│       │   ├── SerializationManager.java # Binary *.gym save/load engine
│       │   ├── InsertDialog.java         # Modal dialog for adding athletes
│       │   ├── EditDialog.java           # Modal dialog for editing athletes
│       │   ├── SearchDialog.java         # Modal dialog for searching athletes
│       │   └── FilterDialog.java         # Stream API multi-criteria sorting table
│       └── resources/gymworld/           # Application graphical resources & stylesheet
│           ├── *.png                     # Sprites, textures, and status badges
│           └── style.css                 # JavaFX dark stylesheet
└── docs/                                 # 📑 Academic documentation & reports
    ├── COURSEWORK-EN.pdf                 # Coursework Explanatory Note (English, 49 pages)
    ├── COURSEWORK-UA.pdf                 # Coursework Explanatory Note (Ukrainian, 49 pages)
    ├── guidelines-for-coursework-EN.pdf  # Methodological Guidelines (English)
    ├── guidelines-for-coursework-UA.pdf  # Methodological Guidelines (Ukrainian)
    ├── OOP-SychYaroslav-2PI-25B.docx     # Editable Word coursework manuscript
    └── assets/                           # UML diagrams & architectural illustrations
        ├── .UML-diagram-3.1.png … 3.7.png# UML diagrams (Inheritance, Sequence, State, etc.)
        └── *.png                         # Documentation asset illustrations
```

---

<br />

---

<a id="-українська-версія"></a>

# 🇺🇦 Українська версія

> 🌐 **Мова:** Українська &nbsp;|&nbsp; **[🇬🇧 Switch to English Version / Перейти до англійської версії](#-english-version)**

---

> [!NOTE]
> **Повноцінний академічний та програмний архів**  
> У цьому каталозі розміщено як **повноцінний графічний симулятор-гру** (**TORNADO GYM Simulator**), розроблений мовою Java 17+ на базі JavaFX з відкритим вихідним кодом, так і повний комплект **академічної документації курсового проєкту** (49-сторінкова пояснювальна записка, комплекс UML-діаграм та методичні вказівки кафедри у папці `docs/`).

---

## 📑 Склад курсового проєкту та документація

| Складова / Документ | Формат | Опис та пряме посилання |
| :----------------------------------- | :----: | :---------------------------------------------------------------------------------------------------------------------------- |
| **Гра-застосунок (Симулятор)**       | `JavaFX` | 🎮 Повний вихідний код десктопного застосунку в [`src/main/java/gymworld/`](./src/main/java/gymworld/) (запуск: `./mvnw clean javafx:run`) |
| **Пояснювальна записка до курсової** | `PDF`  | 🇺🇦 [COURSEWORK-UA.pdf](./docs/COURSEWORK-UA.pdf) — Повна пояснювальна записка до курсової роботи (49 стор.) |
| **Методичні вказівки кафедри**       | `PDF`  | 🇺🇦 [guidelines-for-coursework-UA.pdf](./docs/guidelines-for-coursework-UA.pdf) — Офіційні методичні вказівки ВНТУ з дисципліни ООП |
| **Редагований файл записки**         | `DOCX` | 📄 [OOP-SychYaroslav-2PI-25B.docx](./docs/OOP-SychYaroslav-2PI-25B.docx) — Повний рукопис записки у форматі Microsoft Word |

---

## 🎓 Академічний профіль та метадані

- **Заклад вищої освіти:** Вінницький національний технічний університет ([ВНТУ](https://vntu.edu.ua/))
- **Факультет:** Факультет інформаційних технологій та комп'ютерної інженерії (ФІТКІ)
- **Кафедра:** Кафедра програмного забезпечення
- **Спеціальність:** F2 «Інженерія програмного забезпечення»
- **Дисципліна:** «Об'єктно-орієнтоване програмування» (ООП)
- **Виконавець:** студент групи 2ПІ-25б, **Сич Ярослав Олександрович**
- **Керівник проєкту:** к.т.н., доцент кафедри ПЗ **Решетнік Олександр Олександрович**
- **Підсумкова оцінка:** **82 бали** (Оцінка ECTS: **B**)
- **Рік та місце виконання:** Вінниця, 2025

---

## 🎯 Тема, мета та завдання курсового проєкту

### Тема роботи

> **«Розробка програмної системи моделювання об'єктів: «Симулятор спортзалу TORNADO GYM. Макрооб'єкти: Front Desk & Intake, Cardio Area, Weights Area. Мікрооб'єкти: Athlete, Power Lifter, Gym Boss» з використанням UML та мови програмування Java»**

### Актуальність та соціальна значущість

За даними Всесвітньої організації охорони здоров'я (ВООЗ), понад **1.4 мільярда дорослих у світі** мають недостатній рівень фізичної активності. Розроблений інтерактивний тренажер-симулятор **Tornado GYM** вирішує проблему популяризації спорту за допомогою механік гейміфікації (візуальна гіпертрофія, шкала витривалості, рівні прогресу та позитивне підкріплення), що заохочує молодь до здорового способу життя.

### Основні завдання проєкту

1. Дослідити предметну область (організацію тренувального процесу та взаємодію відвідувачів у фітнес-центрі).
2. Скласти та затвердити технічне завдання (ТЗ) на розробку програмного продукту.
3. Побудувати повний комплекс **UML-моделей**: діаграми успадкування, агрегації, композиції, асоціації, кооперації, послідовності та станів.
4. Розробити графічний кросплатформний застосунок мовою **Java 17+** на базі технології **JavaFX**.
5. Реалізувати динамічну поведінку мікрооб'єктів (3-рівнева ієрархія), механіку зон тренувань (макрооб'єкти), навігацію з міні-картою, збереження стану (серіалізація `.gym`), а також складні фільтри з використанням Stream API та Lambda.
6. Провести тестування функціональності та підготувати інструкцію користувача.

---

## 🚀 Як запустити гру-застосунок

### Попередні вимоги

- **Java Development Kit (JDK):** версія 17 або новіша (перевірено на JDK 21).
- **Maven:** необов'язково (у репозиторії вже налаштовано кросплатформний Maven Wrapper `mvnw` / `mvnw.cmd`, тому окреме встановлення Maven не потрібне).

### Запуск через термінал (Maven Wrapper)

Перейдіть у каталог цього проєкту (`course-projects/OOP-java`) та виконайте команду:

```bash
# Для Linux / macOS:
./mvnw clean javafx:run

# Для Windows (PowerShell або CMD):
.\mvnw.cmd clean javafx:run
```

### Запуск у середовищі розробки (IntelliJ IDEA / VS Code / Eclipse)

1. Відкрийте папку `course-projects/OOP-java` у вашій IDE як Maven проєкт.
2. Дочекайтеся індексації та завантаження залежностей із файлу [`pom.xml`](./pom.xml).
3. Запустіть метод `main()` у класі [`src/main/java/gymworld/Launcher.java`](./src/main/java/gymworld/Launcher.java) або безпосередньо у [`src/main/java/gymworld/GymWorld.java`](./src/main/java/gymworld/GymWorld.java).

---

## 💻 Архітектура програмного коду (Пакет gymworld)

Уся бізнес-логіка, графічні вузли та контролери застосунку структуровані в пакеті `gymworld` ([`src/main/java/gymworld/`](./src/main/java/gymworld/)):

| Компонент / Клас | Роль та функціонал у симуляції |
| :--- | :--- |
| [`Launcher.java`](./src/main/java/gymworld/Launcher.java) | Точка входу в програму. Запускає `GymWorld.class`, обходячи обмеження модульної системи JavaFX без необхідності створення `module-info.java`. |
| [`GymWorld.java`](./src/main/java/gymworld/GymWorld.java) | Головне вікно та серце симуляції. Містить ігровий цикл на базі `AnimationTimer` (60 FPS), камеру вікна перегляду (динамічний Viewport), обробники клавіатури й миші, систему виявлення колізій та переміщення об'єктів. |
| [`Athlete.java`](./src/main/java/gymworld/Athlete.java) | Базовий клас мікрооб'єкта (відвідувач). Реалізує механіку витривалості (1–100), витрату енергії при тренуваннях, смужку здоров'я (`energyBar`), детекцію зон та глибоке клонування (`Cloneable`). |
| [`PowerLifter.java`](./src/main/java/gymworld/PowerLifter.java) | Клас важкоатлета (нащадок `Athlete`). Перевизначає метод `train()` із підвищеною силою та витратою сил; веде облік персональних силових рекордів (жим, тяга, присід). |
| [`GymBoss.java`](./src/main/java/gymworld/GymBoss.java) | Елітний персонаж — бос спортзалу (нащадок `PowerLifter`). Володіє унікальною поліморфною аурою `motivate()`, яка відновлює енергію спортсменів поблизу. |
| [`TrainingZone.java`](./src/main/java/gymworld/TrainingZone.java) | Макрооб'єкти тренувальних зон (Front Desk, Cardio Area, Weights Area). Перевіряє перетин меж об'єктів (`intersects()`), рахує кількість відвідувачів та оновлює індикатори. |
| [`Equipment.java`](./src/main/java/gymworld/Equipment.java) | Модель спортивного спорядження та інвентарю, що закріплюється за атлетами. |
| [`Minimap.java`](./src/main/java/gymworld/Minimap.java) | Інтерактивна міні-карта (радар) у лівому верхньому кутку. Відображає масштабне положення зон і атлетів; клік по карті миттєво центрує камеру на вибраній ділянці залу. |
| [`SerializationManager.java`](./src/main/java/gymworld/SerializationManager.java) | Підсистема збереження стану. Здійснює двійкову серіалізацію та десеріалізацію знімка симуляції у файли `.gym` за допомогою `ObjectOutputStream`/`ObjectInputStream` з конструкцією `try-with-resources`. |
| [`InsertDialog.java`](./src/main/java/gymworld/InsertDialog.java) | Модальне вікно створення нового атлета (<kbd>Insert</kbd>) з налаштуванням типу, імені, ваги та початкових показників. |
| [`EditDialog.java`](./src/main/java/gymworld/EditDialog.java) | Контекстне модальне вікно редагування вибраного атлета (<kbd>ПКМ</kbd>). |
| [`SearchDialog.java`](./src/main/java/gymworld/SearchDialog.java) | Модальне вікно параметричного пошуку атлетів у середовищі тренажерного залу. |
| [`FilterDialog.java`](./src/main/java/gymworld/FilterDialog.java) | Вікно розширеної аналітики (<kbd>G</kbd>) з таблицею даних, багатокритеріальним сортуванням та фільтрацією на основі Java Stream API. |

### Графічні ресурси та стилі

Розміщені у каталозі [`src/main/resources/gymworld/`](./src/main/resources/gymworld/):
- **Спрайти та текстури:** `athlete.png`, `power_lifter.png`, `gym_boss.png`, `front_desk.png`, `cardio_area.png`, `weights_area.png`, бейджі активності (`bicep.png`, `lightning.png`, `wave.png`).
- **Стилізація інтерфейсу:** `style.css` (темна тема оформлення вікон, шрифти, акцентні кольори для кнопок і радарів).

---

## 🏗️ Архітектура предметної області (Макро- та мікрооб'єкти)

Простір симуляції моделює тренажерний зал `GymWorld` із двовимірною системою координат, динамічним оновленням через `AnimationTimer` та підтримкою колізій.

### 1. Мікрооб'єкти (3-рівнева ієрархія успадкування)

- **[`Athlete`](./docs/assets/athlete-trimmed.png) (Базовий клас):**
  - Звичайний відвідувач спортивного залу (чорна футболка з логотипом Tornado GYM, шорти).
  - **Поля:** `name` (ім'я), `weight` (вага), `energyLvl` (рівень витривалості 1–100), `equipment` (екіпірування), `x`, `y`, `isActive`, `currentZone`.
  - **Графічні елементи:** спрайт персонажа (`ImageView`), текстові підписи, графічна смужка запасу сил (`energyBar`), райдужна рамка виділення (`activeBorder`), тематична іконка поточної зони.
  - **Поведінка:** переміщення сценою, витрата витривалості під час тренування, відновлення через `rest()` та `eat()`, підтримка глибокого копіювання (`clone()`).
- **[`Power Lifter`](./docs/assets/athlete-power-lifter-trimmed.png) (Клас-нащадок `Athlete`):**
  - Досвідчений атлет силових дисциплін (синій комбінезон пауерліфтера).
  - **Додаткові поля:** `maxBenchPress` (жим лежачи), `maxDeadlift` (станова тяга), `maxSquat` (присідання).
  - **Поведінка:** перевизначає метод `train()`, оптимізований для зони важких ваг (Weights Area).
- **[`Gym Boss`](./docs/assets/athlete-gym-boss-trimmed.png) (Клас-нащадок `Power Lifter`):**
  - Еліта та головний авторитет залу (червоний спортивний костюм).
  - **Додаткові поля:** `authorityLevel` (рівень авторитету), `gymMastery` (майстерність).
  - **Поведінка:** реалізує унікальний поліморфний метод `motivate(List<Athlete>)` — бафає витривалість та ефективність усіх атлетів поблизу.

### 2. Макрооб'єкти (Зони тренувань `TrainingZone`)

- **[`Front Desk & Intake`](./docs/assets/front-desk-and-intake-graph.png) (600 × 280 px):** Зона рецепції, вітрини зі спортивним харчуванням, реєстрація клієнтів.
- **[`Cardio Area`](./docs/assets/cardio-area-graph.png) (480 × 320 px):** Кардіо-зона з біговими доріжками, велотренажерами та орбітреками.
- **[`Weights Area`](./docs/assets/weights-area-graph.png) (480 × 320 px):** Зона силових тренувань (гантельний ряд, силові рами, олімпійські грифи, лави для жиму).
- **Особливості:**
  - Автоматична перевірка перетину меж через метод `intersects()`.
  - Динамічний лічильник відвідувачів (`countText`): червоний колір, коли зона порожня (`0`), чорний — коли є відвідувачі.
  - Двосторонній зв'язок (`addAthlete(a)` та `removeAthlete(a)`).

### 3. Універсальний об'єкт середовища (`GymWorld`)

- Головний контейнер сцени з таймером реального часу `AnimationTimer`.
- **Динамічне вікно перегляду:** відображає активний фрагмент залу ($\le 25\%$ від загальної площі карти).
- **Інтерактивна міні-карта:** радар у лівому верхньому куті, що масштабує положення всіх об'єктів. Клік лівою кнопкою миші на міні-карті миттєво центрує камеру на відповідній зоні.

---

## 🧩 Реалізація принципів ООП та шаблонів

| Принцип ООП              | Реалізація у проєкті Tornado GYM                                                                                                                               |
| :----------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Абстракція**           | Виділення сутностей предметної області та чітке розмежування моделі даних і графічних вузлів JavaFX.                                                           |
| **Інкапсуляція**         | Закриття внутрішніх полів модифікаторами `private`/`protected`, доступ через верифіковані геттери та сеттери.                                                  |
| **Успадкування**         | Чітка 3-рівнева ієрархія: `Athlete` $\rightarrow$ `PowerLifter` $\rightarrow$ `GymBoss` із ланцюговим викликом `super()`.                                      |
| **Поліморфізм**          | **Статичний:** перевантаження конструкторів та методів запитів. **Динамічний:** перевизначення `train()` та `motivate()`, використання оператора `instanceof`. |
| **Композиція**           | Клас `TrainingZone` безпосередньо створює та керує життєвим циклом своїх графічних елементів (`ImageView`, `Text`, `Rectangle`, `Group`).                      |
| **Агрегація**            | Клас `GymWorld` агрегує динамічні колекції екземплярів `Athlete` та `TrainingZone`.                                                                            |
| **Асоціація**            | Двонаправлена асоціація між атлетом та тренувальною зоною: `Athlete.currentZone` $\leftrightarrow$ `TrainingZone.zoneAthletes`.                                |
| **Глибоке копіювання**   | Реалізація інтерфейсу `Cloneable` та методу `clone()` для створення незалежних дублікатів об'єктів.                                                            |
| **Серіалізація**         | Збереження стану в двійкові файли `.gym` за допомогою `Serializable`. Графічні вузли позначені як `transient` та ініціалізуються методом `initVisuals()`.      |
| **Stream API та Lambda** | Використання сучасних конструкцій Java для фільтрації списків, сортування за трьома критеріями та виконання аналітичних підрахунків.                           |

---

## 📊 UML-моделювання та діаграми проєкту

Усі діаграми спроєктовані відповідно до стандартів UML та доступні у каталозі [`docs/assets/`](./docs/assets/):

1. **Діаграма успадкування класів** ([`.UML-diagram-3.1.png`](./docs/assets/.UML-diagram-3.1.png)) — 3-рівнева ієрархія мікрооб'єктів.
2. **Діаграма агрегації** ([`.UML-diagram-3.2.png`](./docs/assets/.UML-diagram-3.2.png)) — Зв'язки контейнерів та агрегованих сутностей `GymWorld`.
3. **Діаграма композиції** ([`.UML-diagram-3.3.png`](./docs/assets/.UML-diagram-3.3.png)) — Внутрішня графічна структура класу `TrainingZone`.
4. **Діаграма асоціації** ([`.UML-diagram-3.4.png`](./docs/assets/.UML-diagram-3.4.png)) — Взаємодія `Athlete` $\leftrightarrow$ `TrainingZone`.
5. **Діаграма кооперації (комунікації)** ([`.UML-diagram-3.5.png`](./docs/assets/.UML-diagram-3.5.png)) — Обмін повідомленнями під час колізій та зміни зон.
6. **Діаграма послідовності** ([`.UML-diagram-3.6.png`](./docs/assets/.UML-diagram-3.6.png)) — Хронологічний ланцюжок викликів методів від спрацювання таймера до оновлення GUI.
7. **Діаграма станів** ([`.UML-diagram-3.7.png`](./docs/assets/.UML-diagram-3.7.png)) — Життєвий цикл атлета: `Очікування (Idle)` $\rightarrow$ `Рух (Moving)` $\rightarrow$ `Тренування (Training)` $\rightarrow$ `Виснаження (Exhausted)` $\rightarrow$ `Відпочинок (Resting)`.

---

## 🎮 Керування та гарячі клавіші

|                                               Клавіша / Дія                                               | Назва функції                | Опис дії                                                                                                                                  |
| :-------------------------------------------------------------------------------------------------------: | :--------------------------- | :---------------------------------------------------------------------------------------------------------------------------------------- |
|                                             <kbd>Insert</kbd>                                             | **Додавання атлета**         | Відкриває діалогове вікно (`TextField`, `ChoiceBox`, `RadioButton`) для створення нового персонажа (`Athlete`, `PowerLifter`, `GymBoss`). |
|                                             <kbd>Delete</kbd>                                             | **Видалення атлета**         | Видаляє виділеного активного атлета з карти залу.                                                                                         |
|                                      <kbd>Ctrl</kbd> + <kbd>C</kbd>                                       | **Клонування об'єкта**       | Створює повну незалежну копію активного атлета зі зміщенням праворуч.                                                                     |
|                                      <kbd>Ctrl</kbd> + <kbd>S</kbd>                                       | **Збереження стану**         | Викликає `FileChooser` для експорту поточного стану тренажерного залу у файл `.gym`.                                                      |
|                                      <kbd>Ctrl</kbd> + <kbd>L</kbd>                                       | **Завантаження стану**       | Викликає `FileChooser` для відновлення збереженої симуляції з файлу `.gym`.                                                               |
|                                               <kbd>G</kbd>                                                | **Статистика та сортування** | Відкриває табличне вікно з можливістю сортування за трьома критеріями.                                                                    |
| <kbd>W</kbd> <kbd>A</kbd> <kbd>S</kbd> <kbd>D</kbd> / <kbd>↑</kbd> <kbd>←</kbd> <kbd>↓</kbd> <kbd>→</kbd> | **Керування рухом**          | Переміщує виділеного персонажа картою фітнес-центру.                                                                                      |
|                                              <kbd>ЛКМ</kbd>                                               | **Вибір / Міні-карта**       | Виділення об'єкта / клік по міні-карті для миттєвого переміщення камери.                                                                  |
|                                              <kbd>ПКМ</kbd>                                               | **Редагування атлета**       | Відкриває діалогове вікно модифікації ваги, витривалості та екіпірування об'єкта.                                                         |

### Реалізовані запити та фільтри

- 🔍 **Запит 1:** Пошук мікрооб'єкта за введеними параметрами з виведенням локації та приналежності до макрооб'єкта.
- 📋 **Запит 2:** Виведення списку мікрооб'єктів, які перебувають у вказаній тренувальній зоні.
- 🚫 **Запит 3:** Виведення списку мікрооб'єктів поза межами будь-яких тренувальних зон.
- 🔢 **Кількісні запити:** Підрахунок кількості активних атлетів, атлетів із рівнем енергії $> 50\%$, та атлетів у правій половині поля.
- 🔀 **Сортування за критеріями:** За ім'ям (в алфавітному / зворотному порядку), за вагою, за рівнем витривалості (енергії).

---

## ⚙️ Системні вимоги та стек технологій

### Програмний стек

- **Мова програмування:** Java SE 17+ (LTS)
- **Графічна підсистема:** OpenJFX (JavaFX) 21+ з апаратним прискоренням
- **Система збірки:** Apache Maven (з вбудованим Maven Wrapper `mvnw`)
- **Середовище розробки:** IntelliJ IDEA / Eclipse / NetBeans / VS Code
- **Формат збереження:** Двійкова серіалізація Java Object Serialization (`.gym`)

### Апаратні вимоги

- **Процесор:** IBM PC-сумісний процесор з частотою від 2.2 ГГц.
- **Оперативна пам'ять:** 8 ГБ RAM (програма споживає ~270 МБ).
- **Дисковий простір:** 25 МБ вільного місця.
- **Відеоадаптер:** SVGA графічний адаптер з підтримкою OpenGL / DirectX (від 2 ГБ VRAM).
- **ОС:** Windows 10/11, macOS, Linux.

---

## 📁 Структура каталогу

```text
course-projects/OOP-java/
├── pom.xml                               # Конфігурація проєкту Maven (Java 17, JavaFX 21, JUnit 5)
├── mvnw / mvnw.cmd                       # Кросплатформний Maven Wrapper
├── README.md                             # 📖 Документація проєкту (Bilingual: EN / UA)
├── src/                                  # 🎮 Вихідний код та ресурси гри-застосунку
│   └── main/
│       ├── java/gymworld/                # Вихідний код мовою Java
│       │   ├── Launcher.java             # Точка входу в застосунок
│       │   ├── GymWorld.java             # Головний ігровий цикл, полотно та вікно перегляду
│       │   ├── Athlete.java              # Базова сутність атлета (відвідувач)
│       │   ├── PowerLifter.java          # Клас-нащадок важкоатлета
│       │   ├── GymBoss.java              # Клас-нащадок боса спортзалу
│       │   ├── TrainingZone.java         # Макрооб'єкти зон залу та система колізій
│       │   ├── Equipment.java            # Спортивне спорядження
│       │   ├── Minimap.java              # Інтерактивна міні-карта HUD
│       │   ├── SerializationManager.java # Менеджер серіалізації файлів *.gym
│       │   ├── InsertDialog.java         # Діалог додавання атлета
│       │   ├── EditDialog.java           # Діалог редагування атлета
│       │   ├── SearchDialog.java         # Діалог пошуку атлетів
│       │   └── FilterDialog.java         # Таблиця розширеної аналітики зі Stream API
│       └── resources/gymworld/           # Графічні ресурси та файли стилів
│           ├── *.png                     # Спрайти, текстури зон та бейджі активностей
│           └── style.css                 # CSS-таблиця стилів для JavaFX
└── docs/                                 # 📑 Академічна документація та звіти
    ├── COURSEWORK-EN.pdf                 # Пояснювальна записка (English, 49 стор.)
    ├── COURSEWORK-UA.pdf                 # Пояснювальна записка (Українська, 49 стор.)
    ├── guidelines-for-coursework-EN.pdf  # Методичні вказівки до курсової (English)
    ├── guidelines-for-coursework-UA.pdf  # Методичні вказівки до курсової (Українська)
    ├── OOP-SychYaroslav-2PI-25B.docx     # Редагований рукопис курсової роботи в Word
    └── assets/                           # Комплект діаграм UML та графічні схеми
        ├── .UML-diagram-3.1.png … 3.7.png# Діаграми UML (класів, послідовності тощо)
        └── *.png                         # Графічні ілюстрації та схеми для записки
```
