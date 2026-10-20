<a id="-english-version"></a>

# 🇬🇧 Coursework Explanatory Note & Academic Documentation

> 🌐 **Language:** English &nbsp;|&nbsp; **[🇺🇦 Switch to Ukrainian Version / Перейти до української версії](#-українська-версія)**

---

> [!IMPORTANT]
> **Type of README:** 📑 **Academic Coursework Documentation & Explanatory Note README**  
> This file is dedicated specifically to the **Coursework Explanatory Note, Academic Deliverables, and University Materials** for the course project *"Development of a software system for modeling of objects: «TORNADO GYM Simulator. Macro-objects: Front Desk & Intake, Cardio Area, Weights Area. Micro-objects: Athlete, Power Lifter, Gym Boss» using UML and Java"*: 49-page explanatory note breakdown, chapter summaries, formal UML modeling specifications, university guidelines, and defense evaluation.  
> 
> 🎮 **Looking for the Game Application & Source Code?**  
> See the dedicated **[Game Application & Source Code README (`../README.md`)](../README.md)** for game loop mechanics, user controls, hotkey guide, Maven launch instructions, and domain architecture.

---

## 🎓 Academic Profile & Defense Metadata

| Metric / Parameter | Value / Description |
| :--- | :--- |
| **Institution** | Vinnytsia National Technical University ([VNTU](https://vntu.edu.ua/)) |
| **Faculty** | Faculty of Information Technology and Computer Engineering (FITCE / ФІТКІ) |
| **Department** | Department of Software Engineering (Кафедра ПЗ) |
| **Academic Program / Major** | F2 Software Engineering (121 Інженерія програмного забезпечення) |
| **Course / Discipline** | Object-Oriented Programming (Об'єктно-орієнтоване програмування) |
| **Author** | **Yaroslav SYCH**, Student of Group 2PI-25b (Сич Ярослав Олександрович) |
| **Academic Advisor / Supervisor** | Ph.D. in Engineering, Associate Professor **Oleksandr RESHETNIK** (Решетнік О. О.) |
| **Defense Evaluation** | **82 / 100** (ECTS Grade: **B**, National: Good / Добре) |
| **Term & Year** | Academic Year 2024/2025, Vinnytsia |

---

## 📑 Archive Files & Academic Deliverables

All documentation files located in [`course-projects/OOP-java/docs/`](./) are structured as follows:

| File | Format | Language | Description |
| :--- | :---: | :---: | :--- |
| [`COURSEWORK-EN.pdf`](./COURSEWORK-EN.pdf) | `PDF` | 🇬🇧 English | Complete 49-page final Coursework Explanatory Note (English translation) |
| [`COURSEWORK-UA.pdf`](./COURSEWORK-UA.pdf) | `PDF` | 🇺🇦 Ukrainian | Complete 49-page final Coursework Explanatory Note (Original Ukrainian version) |
| [`OOP-SychYaroslav-2PI-25B.docx`](./OOP-SychYaroslav-2PI-25B.docx) | `DOCX` | 🇺🇦 Ukrainian | Full editable thesis manuscript in Microsoft Word formatted to VNTU standards |
| [`guidelines-for-coursework-EN.pdf`](./guidelines-for-coursework-EN.pdf) | `PDF` | 🇬🇧 English | Official VNTU department methodological guidelines for coursework (English) |
| [`guidelines-for-coursework-UA.pdf`](./guidelines-for-coursework-UA.pdf) | `PDF` | 🇺🇦 Ukrainian | Official VNTU department methodological guidelines for coursework (Ukrainian) |
| [`assets/`](./assets/) | `DIR` | — | UML diagram schematics, micro/macro-object graphics, and department reference theses |
| [`../README.md`](../README.md) | `MD` | 🇬🇧 / 🇺🇦 | Root README for the interactive JavaFX desktop game application |

---

## 📖 Structure & Contents of the Explanatory Note (49 Pages)

The coursework explanatory note is formatted in strict accordance with the Ukrainian Higher Education Standards (DSTU 3008:2015) and VNTU Department of Software Engineering guidelines:

### 1. Title Page, Assignment & Abstract (pp. 1–5)
- Official institutional title page approved by the department.
- Formal Coursework Assignment with individual variant parameters:
  - **Macro-objects:** Front Desk & Intake, Cardio Area, Weights Area.
  - **Micro-objects:** Athlete (regular visitor), Power Lifter (strength athlete), Gym Boss (fitness mentor).
- Structured abstract in Ukrainian and English highlighting project goals, methods, and results.

### 2. Introduction (Вступ, pp. 6–8)
- **Relevance:** Addressing the sedentary lifestyle epidemic (WHO data indicates $>1.4$ billion adults are physically inactive).
- **Gamification as a Solution:** Leveraging positive reinforcement, stamina recovery mechanics, and muscle hypertrophy simulation to cultivate sports motivation.
- **Goals and Tasks:** Formulating architectural requirements, designing 7 UML diagrams, implementing a JavaFX application, and conducting verification.

### 3. Chapter 1: Analytical Review & Technical Specification (Розділ 1, pp. 9–17)
- Comprehensive comparative analysis of existing fitness simulation systems and 2D game loops.
- Formal formulation of the **Technical Specification (TS)**:
  - Multi-level object inheritance hierarchy.
  - Collision detection between micro-objects and macro-zones with dynamic visitor counters.
  - Interactive graphical UI with radar minimap and viewport navigation.
  - Object persistence engine via custom binary serialization (`.gym` format).
  - Stream API analytical querying, multi-criteria sorting, and modal dialog manipulation.

### 4. Chapter 2: Object-Oriented System Design with UML (Розділ 2, pp. 18–28)
Formal system design using Unified Modeling Language (UML) specifications:
- **Class Inheritance Diagram:** Models 3-tier micro-object hierarchy (`Athlete` $\rightarrow$ `PowerLifter` $\rightarrow$ `GymBoss`).
- **Aggregation Diagram:** Structural aggregation of `Athlete` and `TrainingZone` collections within `GymWorld`.
- **Composition Diagram:** Lifecycle composition of graphical JavaFX nodes (`Rectangle`, `ImageView`, `Text`) within `TrainingZone`.
- **Association Diagram:** Bidirectional mapping and reference synchronization between athletes and training zones.
- **Cooperation / Communication Diagram:** Runtime message passing during collision detection and zone occupancy changes.
- **Sequence Diagram:** Chronological interaction flow upon an athlete entering a zone (boundary intersection $\rightarrow$ zone counter increment $\rightarrow$ status badge update).
- **State Machine Diagram:** Character behavioral lifecycle (`Idle` $\rightarrow$ `Moving` $\rightarrow$ `Training` $\rightarrow$ `Exhausted` $\rightarrow$ `Resting`).

### 5. Chapter 3: Software Implementation & Algorithms (Розділ 3, pp. 29–39)
- Modular software architecture with clean domain separation in package `gymworld`.
- Practical application of OOP pillars:
  - **Abstraction:** Separation of mathematical domain logic from JavaFX rendering.
  - **Encapsulation:** Guarded fields, input validation, state consistency.
  - **Inheritance:** Extensible character classes with constructor chaining via `super()`.
  - **Polymorphism:** Method overriding (`train()`, `motivate()`) and query overloads.
- State persistence subsystem (`SerializationManager.java`): binary serialization using `ObjectOutputStream` / `ObjectInputStream` with `transient` GUI fields and post-load visual reinitialization.
- Analytical reporting engine (`FilterDialog.java`): dynamic multi-criteria filtering using Java 17 Stream API and Lambdas.

### 6. Chapter 4: Testing & Verification (Розділ 4, pp. 40–45)
- Unit testing strategy covering core mathematical and domain invariants.
- Boundary condition validation: stamina bounds $[0, 100]$, viewport clamping, collision edge cases.
- Performance profiling: consistent 60 FPS under active simulation with memory footprint $\approx 270$ MB.

### 7. Conclusions, References & Appendices (pp. 46–49)
- Summary of theoretical and practical results.
- 15+ academic references according to DSTU 8302:2015.
- Appendices containing key source code listings and graphical asset catalogs.

---

## 📊 UML Modeling Specifications Catalog

The explanatory note includes 7 formal UML diagrams, all stored as visual assets in [`assets/`](./assets/):

| Diagram | Type | Asset File | Academic Purpose & Description |
| :--- | :---: | :---: | :--- |
| **3.1** | **Class (Inheritance)** | [`.UML-diagram-3.1.png`](./assets/.UML-diagram-3.1.png) | 3-tier hierarchy: `Athlete` $\rightarrow$ `PowerLifter` $\rightarrow$ `GymBoss` with full fields and methods |
| **3.2** | **Aggregation** | [`.UML-diagram-3.2.png`](./assets/.UML-diagram-3.2.png) | Container relationship showing `GymWorld` managing collections of zones and visitors |
| **3.3** | **Composition** | [`.UML-diagram-3.3.png`](./assets/.UML-diagram-3.3.png) | Visual node composition inside `TrainingZone` (`ImageView`, `Text`, `Rectangle`, `Group`) |
| **3.4** | **Association** | [`.UML-diagram-3.4.png`](./assets/.UML-diagram-3.4.png) | Bidirectional link: `Athlete.currentZone` $\leftrightarrow$ `TrainingZone.zoneAthletes` |
| **3.5** | **Cooperation** | [`.UML-diagram-3.5.png`](./assets/.UML-diagram-3.5.png) | Inter-object collaboration during coordinate boundary collision checks |
| **3.6** | **Sequence** | [`.UML-diagram-3.6.png`](./assets/.UML-diagram-3.6.png) | Time-ordered message flow on zone entry, counter increment, and UI indicator update |
| **3.7** | **State Machine** | [`.UML-diagram-3.7.png`](./assets/.UML-diagram-3.7.png) | Finite state machine modeling athlete stamina states (`Idle`, `Moving`, `Training`, etc.) |

---

## 🎨 Architectural & Graphic Assets in `docs/assets/`

The [`assets/`](./assets/) subfolder also provides graphical materials used in the explanatory note illustrations:

- **Micro-Object Models:** Regular visitor ([`athlete-trimmed.png`](./assets/athlete-trimmed.png)), Power Lifter ([`athlete-power-lifter-trimmed.png`](./assets/athlete-power-lifter-trimmed.png)), and Gym Boss ([`athlete-gym-boss-trimmed.png`](./assets/athlete-gym-boss-trimmed.png)).
- **Macro-Object Models:** Front Desk ([`front-desk-and-intake-graph.png`](./assets/front-desk-and-intake-graph.png)), Cardio Area ([`cardio-area-graph.png`](./assets/cardio-area-graph.png)), and Weights Area ([`weights-area-graph.png`](./assets/weights-area-graph.png)).
- **University Reference Theses:** Historical sample coursework documents from the department (`isakov-andrew-*.docx`, `oop-nechuj-levytskij-*.docx`, `oop-vovchok-*.docx`, `temi-2025-final.pdf`).

---

<br />

---

<a id="-українська-версія"></a>

# 🇺🇦 Пояснювальна записка до курсової роботи та академічна документація

> 🌐 **Мова:** Українська &nbsp;|&nbsp; **[🇬🇧 Switch to English Version / Перейти до англійської версії](#-english-version)**

---

> [!IMPORTANT]
> **Тип README-файлу:** 📑 **Документація до пояснювальної записки та матеріалів курсової роботи**  
> Цей файл присвячено виключно **пояснювальній записці, академічним результатам та методичним матеріалам** курсового проєкту з дисципліни «Об'єктно-орієнтоване програмування» на тему: *«Розробка програмної системи для моделювання об'єктів: «TORNADO GYM Simulator. Макрооб'єкти: Front Desk & Intake, Cardio Area, Weights Area. Мікрооб'єкти: Athlete, Power Lifter, Gym Boss» засобами UML та мови програмування Java»*: змісту 49-сторінкової записки, анотаціям розділів, специфікаціям UML-діаграм, методичним вказівкам ВНТУ та відомостям про захист.  
> 
> 🎮 **Шукаєте гру, вихідний код та інструкції з запуску симулятора?**  
> Перейдіть до головного **[README ігрового застосунку (`../README.md`)](../README.md)**, де описано запуск через Maven, ігровий цикл, керування, гарячі клавіші та архітектуру класів.

---

## 🎓 Академічний профіль та відомості про захист

| Параметр / Показник | Значення / Опис |
| :--- | :--- |
| **Заклад вищої освіти** | Вінницький національний технічний університет ([ВНТУ](https://vntu.edu.ua/)) |
| **Факультет** | Факультет інформаційних технологій та комп'ютерної інженерії (ФІТКІ) |
| **Кафедра** | Кафедра програмного забезпечення (ПЗ) |
| **Спеціальність** | F2 «Інженерія програмного забезпечення» (121) |
| **Дисципліна** | «Об'єктно-орієнтоване програмування» (ООП) |
| **Виконавець** | студент групи 2ПІ-25б, **Сич Ярослав Олександрович** |
| **Керівник проєкту** | к.т.н., доцент кафедри ПЗ **Решетнік Олександр Олександрович** |
| **Результат захисту** | **82 / 100** (Оцінка ECTS: **B**, Національна шкала: **Добре**) |
| **Рік та місто** | Вінниця, 2025 |

---

## 📑 Склад архіву та академічні матеріали

Усі матеріали каталогу [`course-projects/OOP-java/docs/`](./) структуровані наступним чином:

| Файл / Каталог | Формат | Мова | Призначення та зміст |
| :--- | :---: | :---: | :--- |
| [`COURSEWORK-UA.pdf`](./COURSEWORK-UA.pdf) | `PDF` | 🇺🇦 Українська | Повна чистова пояснювальна записка до курсової роботи (49 сторінок) |
| [`COURSEWORK-EN.pdf`](./COURSEWORK-EN.pdf) | `PDF` | 🇬🇧 Англійська | Повна пояснювальна записка, перекладена англійською мовою (49 сторінок) |
| [`OOP-SychYaroslav-2PI-25B.docx`](./OOP-SychYaroslav-2PI-25B.docx) | `DOCX` | 🇺🇦 Українська | Редагований оригінальний рукопис записки у форматі Microsoft Word |
| [`guidelines-for-coursework-UA.pdf`](./guidelines-for-coursework-UA.pdf) | `PDF` | 🇺🇦 Українська | Офіційні методичні вказівки кафедри ПЗ ВНТУ до виконання курсових робіт |
| [`guidelines-for-coursework-EN.pdf`](./guidelines-for-coursework-EN.pdf) | `PDF` | 🇬🇧 Англійська | Методичні вказівки кафедри ПЗ ВНТУ (англійський переклад) |
| [`assets/`](./assets/) | `DIR` | — | Комплект із 7 UML-діаграм, графіка макро/мікрооб'єктів та кафедральні зразки |
| [`../README.md`](../README.md) | `MD` | 🇬🇧 / 🇺🇦 | Головний README з інструкціями до комп'ютерної гри-симулятора |

---

## 📖 Структура та зміст пояснювальної записки (49 сторінок)

Пояснювальна записка оформлена за стандартами вищої школи України (ДСТУ 3008:2015) та методичними вимогами кафедри ПЗ ВНТУ:

### 1. Титульний аркуш, завдання та анотація (с. 1–5)
- Затверджений кафедрою титульний аркуш.
- Офіційне індивідуальне завдання з параметрами індивідуального варіанта:
  - **Макрооб'єкти:** Front Desk & Intake (рецепція), Cardio Area (кардіозона), Weights Area (зона силових тренувань).
  - **Мікрооб'єкти:** Athlete (спортсмен-відвідувач), Power Lifter (силовик), Gym Boss (елітний наставник).
- Структурована двомовна анотація (українською та англійською мовами).

### 2. Вступ (с. 6–8)
- **Актуальність теми:** Проблема гіподинамії в сучасному урбанізованому світі (за даними ВООЗ понад 1,4 млрд дорослих мають дефіцит фізичної активності).
- **Гейміфікація як спосіб розв'язання:** Використання механік позитивного підкріплення, гіпертрофії м'язів та динамічного відновлення витривалості для формування мотивації до спорту.
- **Мета і завдання проєкту:** Формулювання архітектурних вимог, побудова 7 діаграм UML, реалізація на JavaFX та верифікація програмної системи.

### 3. Розділ 1: Аналітичний огляд предметної області та технічне завдання (с. 9–17)
- Порівняльний аналіз сучасних підходів до 2D-ігрових симуляторів та навчальних спортивних систем.
- Формулювання повного **Технічного завдання (ТЗ)**:
  - Багаторівнева ієрархія класів сутностей предметної області.
  - Механізм детекції колізій між персонажами та зонами із динамічним підрахунком відвідувачів.
  - Інтерактивне візуальне середовище з міні-картою та керуванням камерою.
  - Збереження стану світу у власному бінарному форматі `.gym`.
  - Аналітична фільтрація, сортування через Stream API та система модальних діалогових вікон.

### 4. Розділ 2: Об'єктно-орієнтоване проєктування системи засобами UML (с. 18–28)
Комплексне візуальне моделювання системи згідно нотації UML:
- **Діаграма класів (успадкування):** 3-рівнева ієрархія мікрооб'єктів (`Athlete` $\rightarrow$ `PowerLifter` $\rightarrow$ `GymBoss`).
- **Діаграма агрегації:** Структура агрегації колекцій зон і спортсменів об'єктом `GymWorld`.
- **Діаграма композиції:** Життєвий цикл внутрішніх графічних примітивів JavaFX (`Rectangle`, `ImageView`, `Text`) у складі `TrainingZone`.
- **Діаграма асоціації:** Двосторонній взаємозв'язок і синхронізація посилань спортсмена та зони перебування.
- **Діаграма кооперації (комунікації):** Обмін повідомленнями між об'єктами при детекції колізії та перетині меж зони.
- **Діаграма послідовності:** Хронологічний ланцюжок викликів методів при вході спортсмена в тренувальну зону.
- **Діаграма станів:** Модель скінченного автомата життєвого циклу персонажа (`Idle` $\rightarrow$ `Moving` $\rightarrow$ `Training` $\rightarrow$ `Exhausted` $\rightarrow$ `Resting`).

### 5. Розділ 3: Програмна реалізація симулятора (с. 29–39)
- Модульна архітектура проекту в пакеті `gymworld`.
- Реалізація фундаментальних парадигм ООП:
  - **Абстракція:** Розмежування предметної математичної логіки та вузлів сцени JavaFX.
  - **Інкапсуляція:** Закриті модифікатори доступу, валідація у сеттерах, збереження інваріантів стану.
  - **Успадкування:** Спадкування поведінки та атрибутів із ланцюжком конструкторів `super()`.
  - **Поліморфізм:** Перевизначення методів тренування `train()` та мотивації `motivate()`.
- Підсистема серіалізації (`SerializationManager.java`): збереження бінарного образу світу, обробка `transient` полів графіки та їх реініціалізація `initVisuals()`.
- Аналітична таблиця статистики (`FilterDialog.java`): багатопараметричне сортування засобами Java 17 Stream API.

### 6. Розділ 4: Тестування та верифікація програмних засобів (с. 40–45)
- Модульне тестування бізнес-правил та поведінки мікрооб'єктів.
- Перевірка граничних умов: діапазон енергії $[0, 100]$, вихід за межі ігрового поля, границі зон.
- Профілювання швидкодії: стабільні 60 FPS, навантажувальне тестування, споживання пам'яті $\approx 270$ МБ.

### 7. Висновки, список літератури та додатки (с. 46–49)
- Підсумок досягнутих академічних та практичних результатів.
- Список літературних джерел (15 найменувань за ДСТУ 8302:2015).
- Додатки з вихідним кодом ключових компонентів та графічними схемами.

---

## 📊 Каталог специфікацій UML-моделювання

У пояснювальній записці розроблено 7 обов'язкових діаграм UML, файли яких збережено у папці [`assets/`](./assets/):

| Номер | Тип діаграми | Файл у `assets/` | Призначення та зв'язок із запискою |
| :---: | :--- | :---: | :--- |
| **3.1** | **Діаграма класів (успадкування)** | [`.UML-diagram-3.1.png`](./assets/.UML-diagram-3.1.png) | 3-рівнева ієрархія: `Athlete` $\rightarrow$ `PowerLifter` $\rightarrow$ `GymBoss` із полями та методами |
| **3.2** | **Діаграма агрегації** | [`.UML-diagram-3.2.png`](./assets/.UML-diagram-3.2.png) | Зв'язок контейнера: агрегація списків зон і відвідувачів у `GymWorld` |
| **3.3** | **Діаграма композиції** | [`.UML-diagram-3.3.png`](./assets/.UML-diagram-3.3.png) | Композиція графічних вузлів JavaFX всередині об'єкта `TrainingZone` |
| **3.4** | **Діаграма асоціації** | [`.UML-diagram-3.4.png`](./assets/.UML-diagram-3.4.png) | Двосторонній зв'язок: `Athlete.currentZone` $\leftrightarrow$ `TrainingZone.zoneAthletes` |
| **3.5** | **Діаграма кооперації** | [`.UML-diagram-3.5.png`](./assets/.UML-diagram-3.5.png) | Взаємодія об'єктів під час перевірки перетину координат персонажа і зони |
| **3.6** | **Діаграма послідовності** | [`.UML-diagram-3.6.png`](./assets/.UML-diagram-3.6.png) | Часова послідовність викликів: вхід у зону, зміна лічильника, оновлення індикатора |
| **3.7** | **Діаграма станів** | [`.UML-diagram-3.7.png`](./assets/.UML-diagram-3.7.png) | Скінченний автомат переходів стану втоми та активності спортсмена |

---

## 🎨 Графічні матеріали та зразки у папці `docs/assets/`

У каталозі [`assets/`](./assets/) також розміщено допоміжні матеріали пояснювальної записки:

- **Графічні моделі мікрооб'єктів:** Звичайний відвідувач ([`athlete-trimmed.png`](./assets/athlete-trimmed.png)), Силовик ([`athlete-power-lifter-trimmed.png`](./assets/athlete-power-lifter-trimmed.png)) та Джим Бос ([`athlete-gym-boss-trimmed.png`](./assets/athlete-gym-boss-trimmed.png)).
- **Графічні моделі макрооб'єктів:** Рецепція ([`front-desk-and-intake-graph.png`](./assets/front-desk-and-intake-graph.png)), Кардіозона ([`cardio-area-graph.png`](./assets/cardio-area-graph.png)) та Зона силових вправ ([`weights-area-graph.png`](./assets/weights-area-graph.png)).
- **Кафедральні зразки робіт:** Зразки оформлення курсових робіт кафедри ПЗ ВНТУ минулих років (`isakov-andrew-*.docx`, `oop-nechuj-levytskij-*.docx`, `oop-vovchok-*.docx`, `temi-2025-final.pdf`).
