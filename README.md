# 🧩 Mini C++ Compiler

A simplified **Mini C++ Compiler** built entirely in **C++**, demonstrating all major phases of a compiler — from **Lexical Analysis** to **Target Code Generation**.  
This project provides a clear, step-by-step understanding of how compilers work internally.

---

## 🚀 Features

✅ Implements all essential compiler phases:
1. **Lexical Analysis (Lexer)** – Breaks source code into tokens  
2. **Syntax Analysis (Parser)** – Builds an Abstract Syntax Tree (AST)  
3. **Semantic Analysis** – Checks for type and declaration errors  
4. **Intermediate Code Generation (ICG)** – Produces Three Address Code (TAC)  
5. **Optimization** – Simplifies TAC (constant folding, redundant code removal)  
6. **Target Code Generation** – Converts optimized code into pseudo assembly  

✅ Supports arithmetic, conditionals, nested `if-else`, and returns  
✅ Detects **syntax and semantic errors** clearly  
✅ Includes **batch testing** and interactive test execution  

---

## ⚙️ How to Run the Compiler

### 🧱 Step 1: Compile
Open **Command Prompt** or **Git Bash** inside the folder and run:
```bash
g++ main.cpp lexer.cpp parser.cpp semantic.cpp icg.cpp opt.cpp codegen.cpp -o mini_compiler
````

### ⚡ Step 2: Run

To compile a specific test file:

```bash
mini_compiler tests/test01_valid_basic.txt
```

### 💡 Step 3: (Optional) Run via Batch Files

You can simply **double-click**:

* `run_tests.bat` → Runs *all* test cases automatically
* `run_compiler.bat` → Lets you choose one test interactively

No need to type commands manually!

---

## 🧾 Folder Structure

```
mini_cpp_compiler/
│
├── main.cpp                   → Compiler driver (controls all phases)
├── lexer.cpp / lexer.h         → Lexical Analyzer
├── parser.cpp / parser.h       → Syntax Analyzer (AST builder)
├── semantic.cpp / semantic.h   → Semantic Analyzer
├── icg.cpp / icg.h             → Intermediate Code Generator
├── opt.cpp / opt.h             → Optimizer (constant folding, simplification)
├── codegen.cpp / codegen.h     → Pseudo assembly code generator
│
├── token.h                     → Token structure
├── tests/                      → Folder containing test programs
│     ├── test01_valid_basic.txt
│     ├── test02_valid_nested.txt
│     ├── ...
│
├── run_tests.bat               → Batch script to run all test cases
├── run_compiler.bat            → Interactive batch runner
├── .gitignore                  → Ignore compiled/exe files
└── README.md                   → Project documentation
```

---

## 🧪 Example Output

**Input:**

```cpp
int main() {
  int a = 2 + 3;
  int b = a + 4;
  return b;
}
```

**Compiler Output (simplified):**

```
===== INTERMEDIATE CODE (After Optimization) =====
a = 5
b = a + 4
return b

===== TARGET PSEUDO-ASSEMBLY =====
MOV a, 5
MOV r1, a
ADD r1, 4
MOV b, r1
MOV ret, b
RETURN
```

---

## ⚠️ Notes

* Requires **MinGW** or **g++** to compile.
* Tested on **Windows 10/11** (works on Linux/macOS as well).
* `.gitignore` excludes `.exe` and temporary files from Git tracking.
* Each compiler phase prints output clearly in the console.

---

## 🧰 Technologies Used

* **C++17**
* **Object-Oriented Design**
* **Git + GitHub** for version control

---

## 👨‍💻 Authors

Developed by Arzaan Mulla
*Compiler Design Project – 2025*

---

## 🪪 License

This project is licensed under the **MIT License**.
You are free to use, modify, and share it for educational purposes.

---

⭐ *If you found this project helpful, consider giving it a star on GitHub!*

```
