# 📑 Laboratory Work #8: File Input/Output

> **Course:** Programming Fundamentals (OP)  
> **Institution:** Vinnytsia National Technical University (VNTU)  

---

## 🎯 Goal

Master file I/O management in C/C++, file pointer operations (`FILE*`), stream modes (`"r"`, `"w"`, `"a"`), text vs binary file handling, formatted stream reading/writing, and error handling with `EOF`.

---

## 📐 Theoretical Background

Files allow persistent storage of data on disk. In C, file streams are accessed via a `FILE` pointer.

### File Opening Modes (`fopen`)

```c
FILE *fp = fopen("filename.txt", mode);
```

| Mode | Description | Behavior if File Exists / Missing |
| :---: | :--- | :--- |
| `"r"` | Open for reading | Fails (`NULL`) if file does not exist. |
| `"w"` | Open for writing | Overwrites existing file or creates new file. |
| `"a"` | Open for appending | Appends to end of existing file or creates new file. |
| `"rb"` / `"wb"` | Binary read / write | Accesses raw byte data without line translation. |

### File I/O Functions

1. **Formatted Text I/O:**
   - `fprintf(fp, "format", args...)`: Writes formatted data to file.
   - `fscanf(fp, "format", &vars...)`: Reads formatted data from file.
2. **String & Character I/O:**
   - `fgets(buffer, size, fp)`: Safely reads a line (up to `size - 1` chars) from file.
   - `fputs(str, fp)`: Writes a string to file.
   - `fgetc(fp)` / `fputc(ch, fp)`: Character-by-character reading and writing.
3. **Binary I/O:**
   - `fwrite(ptr, size, count, fp)`: Writes binary memory blocks to file.
   - `fread(ptr, size, count, fp)`: Reads binary memory blocks from file.

### Error Handling & EOF Detection

Always verify file initialization and check for End-Of-File (`EOF`):

```c
FILE *fp = fopen("emails.txt", "r");
if (fp == NULL) {
    perror("Error opening file");
    return 1;
}

char line[256];
while (fgets(line, sizeof(line), fp) != NULL) {
    // Process line
}
fclose(fp);
```

---

## 💻 Code & Files

- **Source Code:** [`main.cpp`](./main.cpp)
- **Data File:** [`emails.txt`](./emails.txt)
- **Lab Report:** [UA DOCX](./Звіт_ЛР8_Файлове_введення_виведення.docx)

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
