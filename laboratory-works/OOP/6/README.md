# 📑 Laboratory Work #6

> **Course:** Object-Oriented Programming (OOP)  
> **Institution:** Vinnytsia National Technical University (VNTU)  
> **Language:** English

---

## 🎯 Goal

Implement object serialization and state persistence for the `GymWorld` JavaFX environment, enabling saving and loading entire simulation states into custom binary file packages (`*.gym`) using `ObjectOutputStream` and `ObjectInputStream` with safe resource management (`try-with-resources`).

---

## 💻 Code & Resources

- **Serialization Manager:** [`SerializationManager.java`](./src/main/java/gymworld/SerializationManager.java)
- **Main Application:** [`GymWorld.java`](./src/main/java/gymworld/GymWorld.java)
- **Filter Dialog:** [`FilterDialog.java`](./src/main/java/gymworld/FilterDialog.java)
- **Lab Guidelines:** [Lab 6 Guidelines (PDF)](./ЛР6_Серіалізація_обєктів.pdf)

---

## 🚀 How to Run

Using Maven wrapper:

```bash
./mvnw clean javafx:run
```
