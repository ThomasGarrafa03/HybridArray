# AoS vs SoA - Bachelor Thesis Project

This repository contains the code and resources developed for my **Bachelor's thesis**, focused on exploring **data layout strategies** (Array of Structures vs Structure of Arrays) and their application in **high-performance computing** (HPC) scenarios. The project also includes a **cellular automaton (Sciddica)**, cache coherence resources, and benchmarks to evaluate performance differences.  

## Project Overview

The main goal of this project is to provide a **transparent, dynamic system** to handle AoS/SoA layouts in C++ for HPC applications using **POSIX/OpenMP, CUDA, and MPI**. The dynamic system allows experimentation with different layouts at compile time without changing the user code.  

Key features:
- **Dynamic AoS/SoA layout switching** using templates, **X Macro patterns**, and **Proxy classes**.
- **Benchmarking utilities** to measure average execution time.
- Support for **high-performance computing paradigms** (multithreading, GPU, distributed memory).
- **Cellular automata** experiments (Sciddica) to demonstrate performance impacts of layout choices.

---

## Repository Structure

### Dynamic AoS/SoA System
This part is the core of the project, designed to allow flexible layout switching:

- **`OOpenCALStructs.h`**  
  Implements the generic array abstraction supporting both AoS and SoA.  
  - Uses **X Macro (`Fields.h`)** to define fields.  
  - Uses **`Proxy`** object to transparently access elements regardless of layout.  
  - Manages memory allocation and aligned offsets for efficient access.  

- **`Fields.h`**  
  Lists all fields of the data structure, used by `_OOpenCALArray` via X Macro for automatic generation of getters, setters, and offsets.

- **`ComputeTime.h`**  
  Provides `computeTime` utility to measure **average execution time** of any function or lambda in milliseconds. Useful for benchmarking both AoS and SoA arrays.

- **Benchmark Examples**
  - `SoAoS_Classic.cpp`  
    Measures performance of AoS vs SoA layouts for a simple data structure with multiple fields.  
  - `DynamicAllocation.cpp`  
    Uses the generic `_OOpenCALArray` to benchmark dynamic AoS/SoA arrays with X Macro fields and proxy access.

---

### Cellular Automaton: Sciddica

The repository also contains a **serial implementation of the Sciddica cellular automaton**:

- **`sciddica.cpp`**  
  - Implements a simple cellular automaton simulation.  
  - Supports both **SoA and AoS layouts** with the same template-based design.  
  - Includes **initialization, flow computation, width updates**, and **printing utilities**.  
  - Designed for experimentation with different data layouts and performance evaluation.

- **`Sciddica.h`**, **`SciddicaCell.h`**, **`SciddicaSettings.h`**  
  - Core abstractions for the cellular automaton.  
  - `AbstractSciddica` template allows layout-agnostic access.  
  - Supports **neighbor computations, flows, and cell updates**.  
  - Adjustable parameters: `NROWS`, `NCOLS`, `NSTEPS`, `NUMBER_OF_OUTFLOWS`, `P_EPSILON`, `P_R`.

---

### Additional Resources

- Notes on **Cache coherence**.
- Notes on **AoS vs SoA performance** and HPC memory layouts.
- Benchmarks demonstrating **impact of layout choice** on memory access and execution speed.

---

## Usage

### Dynamic AoS/SoA Benchmark
Compile with either layout:

```bash
# For Structure of Arrays
g++ DynamicAllocation.cpp -D soa -o benchmark_soa.out

# For Array of Structures
g++ DynamicAllocation.cpp -D aos -o benchmark_aos.out
```

Run:

```
./benchmark_soa.out
./benchmark_aos.out
```

Compare them with their "classic" counterpart:
```bash
# For Structure of Arrays
g++ DynamicAllocation.cpp -o benchmark_classic.out
```
Run:
```bash
./benchmark_classic.out
```

### Sciddica Automaton

Compile with layout selection:
```
g++ sciddica.cpp -o sciddica.out
```

Run with layout argument:
```
./sciddica.out --soa
./sciddica.out --aos
```
## Notes

**The dynamic AoS/SoA system is the core innovation of the thesis.** It allows high-performance experiments without modifying the client code, using:
- X Macro for field definitions
- Proxy classe for layout-independent access
- Offset descriptors for SoA memory layout
- All benchmarks measure execution time using computeTime for fair comparison between layouts.

The Sciddica automaton demonstrates how the layout choice affects memory access patterns and performance in a real computational scenario.
