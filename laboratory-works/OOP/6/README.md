# 📑 Laboratory Work #6

> **Course:** Object-Oriented Programming (OOP)  
> **Institution:** Vinnytsia National Technical University (VNTU)  
> **Language:** English

---

> [!NOTE]
> **Source Code Moved to Course Project / Основний код перенесено до курсової роботи**  
> The codebase of this laboratory work was used as the foundation for the OOP Course Project (**TORNADO GYM Simulator**). Consequently, the complete, extended, and actively maintained source code is located in the dedicated course project repository:  
> 🔗 **[`course-projects/OOP-java`](../../../course-projects/OOP-java)**  
>  
> *Код цієї лабораторної роботи (ЛР #6) став основою для курсової роботи з дисципліни «Об'єктно-орієнтоване програмування» (**TORNADO GYM Simulator**). У зв'язку з цим повна, розширена та актуальна версія вихідного коду знаходиться у каталозі курсового проєкту.*

---

## 🎯 Goal & Topic

**Topic:** Object Serialization & System State Persistence (Серіалізація об'єктів та збереження стану системи)

**Objectives:**
- Implement object serialization and state persistence for the `GymWorld` JavaFX environment, enabling saving and loading entire simulation states into custom binary file packages (`*.gym`).
- Utilize standard Java serialization mechanisms (`ObjectOutputStream` and `ObjectInputStream`) to capture complex object graphs (training zones, athletes, equipment, and simulation parameters).
- Ensure safe resource management using `try-with-resources` blocks and proper I/O exception handling.
- Integrate native JavaFX `FileChooser` dialogs for interactive saving and loading of simulation snapshots.

---

## 💻 Code & Resources

The complete source code is part of the OOP Course Project repository:

- 📦 **Course Project Root:** [`course-projects/OOP-java`](../../../course-projects/OOP-java)
- 💾 **Serialization Manager:** [`SerializationManager.java`](../../../course-projects/OOP-java/src/main/java/gymworld/SerializationManager.java)
- 🖥️ **Main JavaFX Application:** [`GymWorld.java`](../../../course-projects/OOP-java/src/main/java/gymworld/GymWorld.java)
- 🔍 **Filter Dialog:** [`FilterDialog.java`](../../../course-projects/OOP-java/src/main/java/gymworld/FilterDialog.java)
- 📑 **Lab Guidelines:** [Lab 6 Guidelines (PDF)](./ЛР6_Серіалізація_обєктів.pdf)

---

## 🚀 How to Run

Navigate to the course project directory and run the application via Maven:

```bash
cd course-projects/OOP-java
./mvnw clean javafx:run
```
