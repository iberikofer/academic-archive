# 📑 Laboratory Work #6: String Processing & Char Arrays

> **Course:** Programming Fundamentals (OP)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Master string processing in C/C++, null-terminated C-style character arrays (`char[]`), standard `<string.h>` / `<cctype>` library functions, string manipulation, and pattern searching algorithms.

---

## 📐 Theoretical Background

In C, strings are represented as contiguous sequences of characters terminated by a null character (`'\0'`, ASCII value `0`).

### C-String Structure & Memory Layout

```c
char str[6] = "Hello"; 
// Memory layout: ['H', 'e', 'l', 'l', 'o', '\0']
```

An array allocated for a string of length $L$ must have a capacity of at least $L + 1$ bytes to store the null terminator.

### Standard String Manipulation Functions (`<string.h>`)

| Function | Signature / Description |
| :--- | :--- |
| `strlen(s)` | Computes the length of string `s` up to (excluding) `'\0'`. |
| `strcpy(dest, src)` | Copies string `src` into buffer `dest` (including `'\0'`). |
| `strcat(dest, src)` | Appends string `src` to the end of string `dest`. |
| `strcmp(s1, s2)` | Returns $< 0$, $0$, or $> 0$ if `s1` is lexicographically less, equal, or greater than `s2`. |
| `strchr(s, ch)` | Returns pointer to first occurrence of character `ch` in `s`. |
| `strstr(s, sub)` | Returns pointer to first occurrence of substring `sub` in `s`. |

### Character Classification (`<ctype.h>`)

- `isalpha(c)`: Checks if `c` is an alphabetic character ($A\text{--}Z, a\text{--}z$).
- `isdigit(c)`: Checks if `c` is a decimal digit ($0\text{--}9$).
- `isspace(c)`: Checks if `c` is a whitespace character (`' '`, `'\t'`, `'\n'`).
- `toupper(c)` / `tolower(c)`: Converts character case.

---

## 💻 Code & Files

- **Source Code:** [`main.cpp`](./main.cpp)
- **Lab Report:** [UA DOCX](./Звіт_ЛР6_Обробка_рядків_та_символів.docx)

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
