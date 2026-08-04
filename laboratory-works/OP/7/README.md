# 📑 Laboratory Work #7: Structures & Enumerations

> **Course:** Programming Fundamentals (OP)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Master user-defined data structures in C/C++, `struct` declarations, member access operators (`.` and `->`), arrays of structures, memory alignment, `union` types, and `enum` enumerations.

---

## 📐 Theoretical Background

A **structure** (`struct`) is a composite user-defined data type that allows grouping variables of different data types into a single named entity.

### Structure Definition & Member Access

```cpp
struct Student {
    char name[50];
    int group_id;
    double gpa;
};

struct Student s1 = {"Alice", 101, 4.85};
```

- **Direct Member Access Operator (`.`):** Used with structure variables: `s1.gpa = 4.90;`
- **Pointer Member Access Operator (`->`):** Used with pointers to structures:

  ```cpp
  struct Student *ptr = &s1;
  ptr->group_id = 102; // Equivalent to (*ptr).group_id
  ```

### Array of Structures

Arrays can store multiple structure instances for record processing (e.g., database simulation):

```cpp
struct Student database[100];
```

### Unions and Enumerations

- **`union`:** A special data type where all members share the exact same memory location. The size of a union is equal to the size of its largest member:

  ```cpp
  union Data {
      int i;
      float f;
      char str[20];
  };
  ```

- **`enum` (Enumeration):** Defines a set of named integer constants:

  ```cpp
  enum Day { MONDAY = 1, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY };
  ```

---

## 💻 Code & Files

- **Source Code:** [`main.cpp`](./main.cpp)
- **Lab Report:** [UA DOCX](./Звіт_ЛР7_Структури_та_переліковні_типи.docx)

---

## 🚀 How to Run

1. Compile C++ source code:

   ```bash
   g++ -O2 main.cpp -o main
   ```

2. Execute binary:

   ```bash
   ./main
   ```
