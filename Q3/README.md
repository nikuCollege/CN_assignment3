# Distance Vector Routing Simulation

A simple simulation of the Distance Vector Routing Algorithm implemented in C.

---

## 📦 Prerequisites

- [GCC (GNU Compiler Collection)](https://gcc.gnu.org/) must be installed on your system.

---

## 📁 Included Files

The following source files should be present in the same directory:

- `distance_vector.c` – Main simulation driver  
- `node0.c` – Logic for Node 0  
- `node1.c` – Logic for Node 1  
- `node2.c` – Logic for Node 2  
- `node3.c` – Logic for Node 3  

---

## 🚀 How to Compile and Run

Open a terminal in the directory containing the files, then run:

```bash
gcc -o routing distance_vector.c node0.c node1.c node2.c node3.c
This will generate an executable file named routing (or routing.exe on Windows).
```

To run the program:

```bash
./routing
```
