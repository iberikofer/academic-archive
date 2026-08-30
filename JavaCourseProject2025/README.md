<a id="-english-version"></a>

# 🇬🇧 English Version

> 🌐 **Language:** English &nbsp;|&nbsp; **[🇺🇦 Switch to Ukrainian Version / Перейти до української версії](#-українська-версія)**

---

## 📑 Project Documentation & Guidelines

| Document                        | Format | Description & Link                                                                                                           |
| :------------------------------ | :----: | :--------------------------------------------------------------------------------------------------------------------------- |
| **Coursework Explanatory Note** | `PDF`  | 🇬🇧 [COURSEWORK-EN.pdf](./COURSEWORK-EN.pdf) — Complete 49-page technical report in English                                   |
| **Methodological Guidelines**   | `PDF`  | 🇬🇧 [guidelines-for-coursework-EN.pdf](./guidelines-for-coursework-EN.pdf) — Official VNTU coursework requirements in English |

---

## 🎓 Academic Profile & Metadata

- **Institution:** Vinnytsia National Technical University ([VNTU](https://vntu.edu.ua/))
- **Faculty:** Faculty of Information Technology and Computer Engineering (FITCE)
- **Department:** Department of Software Engineering
- **Program / Major:** 121 (F2) Software Engineering
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

## 🏗️ Domain Architecture & Model Hierarchy

The simulation takes place within a 2D fitness center environment (`GymWorld`) featuring interactive micro-objects (gym visitors) and macro-objects (training zones).

### 1. Micro-Objects (3-Level Inheritance Hierarchy)

- **[`Athlete`](./assets/athlete-trimmed.png) (Base Class):**
  - Represents a regular gym visitor (black Tornado GYM t-shirt & shorts).
  - **Fields:** `name`, `weight`, `energyLvl` (1–100), `equipment`, `x`, `y`, `isActive`, `currentZone`.
  - **Visuals:** Sprite texture (`ImageView`), status text block, dynamic yellow stamina bar (`energyBar`), activation border (`activeBorder`), zone status indicator.
  - **Behaviors:** Standard navigation, stamina decay during workouts, energy restoration via `rest()` / `eat()`, deep cloning via `Cloneable`.
- **[`Power Lifter`](./assets/athlete-power-lifter-trimmed.png) (Extends `Athlete`):**
  - Experienced heavy strength athlete (blue weightlifting suit).
  - **Additional Fields:** `maxBenchPress`, `maxDeadlift`, `maxSquat`.
  - **Behaviors:** Overrides `train()` with higher stamina consumption and rapid strength scaling, specialized for the Weights Area.
- **[`Gym Boss`](./assets/athlete-gym-boss-trimmed.png) (Extends `Power Lifter`):**
  - Elite authority figure of the gym (red tracksuit).
  - **Additional Fields:** `authorityLevel`, `gymMastery`.
  - **Behaviors:** Unique `motivate(List<Athlete>)` method demonstrating dynamic polymorphism (increases stamina recovery and performance of nearby athletes).

### 2. Macro-Objects (Training Zones)

- **[`Front Desk & Intake`](./assets/front-desk-and-intake-graph.png) (600 × 280 px):** Reception desk with illuminated Tornado logo, protein/supplement displays, visitor check-in.
- **[`Cardio Area`](./assets/cardio-area-graph.png) (480 × 320 px):** Treadmills, stationary bikes, and ellipticals.
- **[`Weights Area`](./assets/weights-area-graph.png) (480 × 320 px):** Power racks, Olympic barbells, dumbbells, and adjustable benches.
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

The project is thoroughly designed and verified against the following UML diagrams (located in [`assets/`](./assets/)):

1. **Class Inheritance Diagram** ([`.UML-diagram-3.1.png`](./assets/.UML-diagram-3.1.png)) — 3-level athlete hierarchy with fields and methods.
2. **Aggregation Diagram** ([`.UML-diagram-3.2.png`](./assets/.UML-diagram-3.2.png)) — Container-component relations between `GymWorld`, `TrainingZone`, and `Athlete`.
3. **Composition Diagram** ([`.UML-diagram-3.3.png`](./assets/.UML-diagram-3.3.png)) — Internal graphical node composition inside `TrainingZone`.
4. **Association Diagram** ([`.UML-diagram-3.4.png`](./assets/.UML-diagram-3.4.png)) — Bidirectional links and reference synchronization.
5. **Cooperation (Communication) Diagram** ([`.UML-diagram-3.5.png`](./assets/.UML-diagram-3.5.png)) — Message passing during collision detection.
6. **Sequence Diagram** ([`.UML-diagram-3.6.png`](./assets/.UML-diagram-3.6.png)) — Chronological sequence of zone entry, counter update, and emoji badge trigger.
7. **State Machine Diagram** ([`.UML-diagram-3.7.png`](./assets/.UML-diagram-3.7.png)) — Life cycle transitions: `Idle` $\rightarrow$ `Moving` $\rightarrow$ `Training` $\rightarrow$ `Exhausted` $\rightarrow$ `Resting`.

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
- **GUI Framework:** OpenJFX (JavaFX) 17+ (Hardware-accelerated rendering)
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
JavaCourseProject2025/
├── COURSEWORK-EN.pdf                 # Coursework Explanatory Note (English)
├── COURSEWORK-UA.pdf                 # Coursework Explanatory Note (Ukrainian)
├── guidelines-for-coursework-EN.pdf  # Methodological Guidelines (English)
├── guidelines-for-coursework-UA.pdf  # Methodological Guidelines (Ukrainian)
├── README.md                         # 📖 Project Documentation (Bilingual: EN / UA)
└── assets/                           # 🎨 Graphical assets, sprites, UML diagrams & samples
    ├── .UML-diagram-3.1.png          # UML Class Inheritance Diagram
    ├── .UML-diagram-3.2.png          # UML Aggregation Diagram
    ├── .UML-diagram-3.3.png          # UML Composition Diagram
    ├── .UML-diagram-3.4.png          # UML Association Diagram
    ├── .UML-diagram-3.5.png          # UML Communication Diagram
    ├── .UML-diagram-3.6.png          # UML Sequence Diagram
    ├── .UML-diagram-3.7.png          # UML State Machine Diagram
    ├── athlete-trimmed.png           # Athlete Sprite
    ├── athlete-power-lifter-trimmed.png # Power Lifter Sprite
    ├── athlete-gym-boss-trimmed.png  # Gym Boss Sprite
    ├── front-desk-and-intake.png     # Front Desk & Intake Sprite
    ├── front-desk-and-intake-graph.png # Front Desk & Intake Area Graph
    ├── cardio-area.png               # Cardio Area Sprite
    ├── cardio-area-graph.png         # Cardio Area Graph
    ├── weights-area.png              # Weights Area Sprite
    └── weights-area-graph.png        # Weights Area Graph
```

---

<br />

---

<a id="-українська-версія"></a>

# 🇺🇦 Українська версія

> 🌐 **Мова:** Українська &nbsp;|&nbsp; **[🇬🇧 Switch to English Version / Перейти до англійської версії](#-english-version)**

---

## 📑 Документація курсового проєкту та методичні матеріали

| Документ                             | Формат | Опис та пряме посилання                                                                                                       |
| :----------------------------------- | :----: | :---------------------------------------------------------------------------------------------------------------------------- |
| **Пояснювальна записка до курсової** | `PDF`  | 🇺🇦 [COURSEWORK-UA.pdf](./COURSEWORK-UA.pdf) — Повна пояснювальна записка до курсової роботи (49 стор.)                        |
| **Методичні вказівки кафедри**       | `PDF`  | 🇺🇦 [guidelines-for-coursework-UA.pdf](./guidelines-for-coursework-UA.pdf) — Офіційні методичні вказівки ВНТУ з дисципліни ООП |

---

## 🎓 Академічний профіль та метадані

- **Заклад вищої освіти:** Вінницький національний технічний університет ([ВНТУ](https://vntu.edu.ua/))
- **Факультет:** Факультет інформаційних технологій та комп'ютерної інженерії (ФІТКІ)
- **Кафедра:** Кафедра програмної інженерії
- **Спеціальність:** 121 (F2) «Інженерія програмного забезпечення»
- **Освітньо-професійна програма:** «Програмна інженерія»
- **Дисципліна:** «Об'єктно-орієнтоване програмування» (ООП)
- **Виконавець:** студент групи 2ПІ-25б, **Сич Ярослав Олександрович**
- **Керівник проєкту:** к.т.н., доцент кафедри ПІ **Решетнік Олександр Олександрович**
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

## 🏗️ Архітектура предметної області (Макро- та мікрооб'єкти)

Простір симуляції моделює тренажерний зал `GymWorld` із двовимірною системою координат, динамічним оновленням через `AnimationTimer` та підтримкою колізій.

### 1. Мікрооб'єкти (3-рівнева ієрархія успадкування)

- **[`Athlete`](./assets/athlete-trimmed.png) (Базовий клас):**
  - Звичайний відвідувач спортивного залу (чорна футболка з логотипом Tornado GYM, шорти).
  - **Поля:** `name` (ім'я), `weight` (вага), `energyLvl` (рівень витривалості 1–100), `equipment` (екіпірування), `x`, `y`, `isActive`, `currentZone`.
  - **Графічні елементи:** спрайт персонажа (`ImageView`), текстові підписи, графічна смужка запасу сил (`energyBar`), райдужна рамка виділення (`activeBorder`), тематична іконка поточної зони.
  - **Поведінка:** переміщення сценою, витрата витривалості під час тренування, відновлення через `rest()` та `eat()`, підтримка глибокого копіювання (`clone()`).
- **[`Power Lifter`](./assets/athlete-power-lifter-trimmed.png) (Клас-нащадок `Athlete`):**
  - Досвідчений атлет силових дисциплін (синій комбінезон пауерліфтера).
  - **Додаткові поля:** `maxBenchPress` (жим лежачи), `maxDeadlift` (станова тяга), `maxSquat` (присідання).
  - **Поведінка:** перевизначає метод `train()`, оптимізований для зони важких ваг (Weights Area).
- **[`Gym Boss`](./assets/athlete-gym-boss-trimmed.png) (Клас-нащадок `Power Lifter`):**
  - Еліта та головний авторитет залу (червоний спортивний костюм).
  - **Додаткові поля:** `authorityLevel` (рівень авторитету), `gymMastery` (майстерність).
  - **Поведінка:** реалізує унікальний поліморфний метод `motivate(List<Athlete>)` — бафає витривалість та ефективність усіх атлетів поблизу.

### 2. Макрооб'єкти (Зони тренувань `TrainingZone`)

- **[`Front Desk & Intake`](./assets/front-desk-and-intake-graph.png) (600 × 280 px):** Зона рецепції, вітрини зі спортивним харчуванням, реєстрація клієнтів.
- **[`Cardio Area`](./assets/cardio-area-graph.png) (480 × 320 px):** Кардіо-зона з біговими доріжками, велотренажерами та орбітреками.
- **[`Weights Area`](./assets/weights-area-graph.png) (480 × 320 px):** Зона силових тренувань (гантельний ряд, силові рами, олімпійські грифи, лави для жиму).
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

Усі діаграми спроєктовані відповідно до стандартів UML та доступні у каталозі [`assets/`](./assets/):

1. **Діаграма успадкування класів** ([`.UML-diagram-3.1.png`](./assets/.UML-diagram-3.1.png)) — 3-рівнева ієрархія мікрооб'єктів.
2. **Діаграма агрегації** ([`.UML-diagram-3.2.png`](./assets/.UML-diagram-3.2.png)) — Зв'язки контейнерів та агрегованих сутностей `GymWorld`.
3. **Діаграма композиції** ([`.UML-diagram-3.3.png`](./assets/.UML-diagram-3.3.png)) — Внутрішня графічна структура класу `TrainingZone`.
4. **Діаграма асоціації** ([`.UML-diagram-3.4.png`](./assets/.UML-diagram-3.4.png)) — Взаємодія `Athlete` $\leftrightarrow$ `TrainingZone`.
5. **Діаграма кооперації (комунікації)** ([`.UML-diagram-3.5.png`](./assets/.UML-diagram-3.5.png)) — Обмін повідомленнями під час колізій та зміни зон.
6. **Діаграма послідовності** ([`.UML-diagram-3.6.png`](./assets/.UML-diagram-3.6.png)) — Хронологічний ланцюжок викликів методів від спрацювання таймера до оновлення GUI.
7. **Діаграма станів** ([`.UML-diagram-3.7.png`](./assets/.UML-diagram-3.7.png)) — Життєвий цикл атлета: `Очікування (Idle)` $\rightarrow$ `Рух (Moving)` $\rightarrow$ `Тренування (Training)` $\rightarrow$ `Виснаження (Exhausted)` $\rightarrow$ `Відпочинок (Resting)`.

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
- **Графічна підсистема:** OpenJFX (JavaFX) 17+ з апаратним прискоренням
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
JavaCourseProject2025/
├── COURSEWORK-EN.pdf                 # Пояснювальна записка до курсової роботи (English)
├── COURSEWORK-UA.pdf                 # Пояснювальна записка до курсової роботи (Українська)
├── guidelines-for-coursework-EN.pdf  # Методичні вказівки до виконання курсової (English)
├── guidelines-for-coursework-UA.pdf  # Методичні вказівки до виконання курсової (Українська)
├── README.md                         # 📖 Документація проєкту (Bilingual: EN / UA)
└── assets/                           # 🎨 Графічні ресурси, спрайти, діаграми UML та зразки
    ├── .UML-diagram-3.1.png          # UML Діаграма класів
    ├── .UML-diagram-3.2.png          # UML Діаграма агрегації
    ├── .UML-diagram-3.3.png          # UML Діаграма композиції
    ├── .UML-diagram-3.4.png          # UML Діаграма асоціації
    ├── .UML-diagram-3.5.png          # UML Діаграма комунікації
    ├── .UML-diagram-3.6.png          # UML Діаграма послідовності
    ├── .UML-diagram-3.7.png          # UML Діаграма станів
    ├── athlete-trimmed.png           # Спрайт атлета
    ├── athlete-power-lifter-trimmed.png # Спрайт важкоатлета
    ├── athlete-gym-boss-trimmed.png  # Спрайт Боса спортзалу
    ├── front-desk-and-intake.png     # Спрайт зони рецепції
    ├── front-desk-and-intake-graph.png # Граф зони рецепції
    ├── cardio-area.png               # Спрайт зони кардіо
    ├── cardio-area-graph.png         # Граф зони кардіо
    ├── weights-area.png              # Спрайт зони важкої атлетики
    └── weights-area-graph.png        # Граф зони важкої атлетики
```
