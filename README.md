# HybridArray
**A Compile-Time AoS / SoA Hybrid Container for High-Performance C++**

HybridArray is a data-oriented C++ container that enables **compile-time selection** between **Array of Structures (AoS)** and **Structure of Arrays (SoA)** memory layouts, while preserving a **single, object-oriented user interface**.

The library is designed to reconcile **hardware-efficient memory layouts** with **usability and STL compatibility**, targeting High Performance Computing (HPC), scientific simulations, and data-parallel workloads.

---

## Motivation

Modern CPU architectures are optimized for:
* Cache locality
* SIMD vectorization
* Predictable and sequential memory access patterns

However, traditional **Object-Oriented Programming (OOP)** naturally leads to **Array of Structures (AoS)** layouts, which often conflict with these hardware characteristics. Conversely, **Structure of Arrays (SoA)** layouts maximize performance but severely degrade usability and maintainability.

HybridArray addresses this dichotomy by offering:

> **One source code → multiple memory layouts → zero runtime overhead**

This approach is rooted in **Data-Oriented Design (DOD)** principles and implemented entirely through **compile-time metaprogramming**.

---

## Key Features

* 🚀 Compile-time AoS / SoA switching
* 🧠 Data-Oriented Design–driven memory layout
* 🧩 Single Source of Truth for data schema (X-Macro based)
* 🪄 Proxy objects enabling `array[i].field` access in SoA
* ⚡ Zero-overhead abstractions via `if constexpr` and NTTPs
* 🔁 STL-compatible proxy iterators
* 📦 Manual lifetime management using placement new
* 🧪 Designed for benchmarking and architectural exploration

---

## High-Level Design

HybridArray is not a traditional container but a **compile-time generated container**.

Its behavior is driven by two mandatory configuration macros:

| Macro | Purpose |
| :--- | :--- |
| `INITFIELDS` | Defines the logical data schema |
| `LAYOUT` | Selects the physical memory layout (AoS / SoA) |

These macros allow the compiler to generate **fully specialized code paths**, completely eliminating runtime branching.

---

## Basic Usage

### 1. Define the Data Schema

```cpp
#define INITFIELDS \
    Field(float, x) \
    Field(float, y) \
    Field(float, z) \
    Field(float, vx) \
    Field(float, vy) \
    Field(float, vz) \
    Field(int, id)
```

This macro represents the **Single Source of Truth** for the data structure. From it, HybridArray generates:
* Memory descriptors
* Proxy objects
* Getters and setters
* Iterators

### 2. Select the Memory Layout (Compile-Time)
The layout is selected via a compiler flag:

```bash
# Structure of Arrays (SIMD- and cache-friendly)
-D LAYOUT=soa

# Array of Structures (object-centric)
-D LAYOUT=aos
```

No source code changes are required.

### 3. Use the Container

```cpp
#include "HybridArray.h"

HybridArray particles(1'000'000);

// Access via index
particles[42].x = 3.14f;
particles[42].vy += 1.0f;

// Access via iterator (range-based for loop)
for (auto& p : particles) {
    p.x += p.vx;
}
```

The user-facing API remains identical regardless of the chosen layout.

---

## Core Concepts

### Compile-Time Control Flow
* The memory layout is encoded as a Non-Type Template Parameter (NTTP).
* `if constexpr` removes unused branches at compile time.
* AoS and SoA become distinct types, not runtime states.
* **Result:** This guarantees zero runtime overhead.

### Proxy Pattern
In a strict SoA layout, a logical object does not exist as a contiguous memory entity. HybridArray solves this by returning a proxy object from `operator[]`.

The proxy:
* Stores references to the internal arrays.
* Stores the accessed index.
* Redirects field access to the correct memory location.

Modern compilers inline and optimize away the proxy entirely, resulting in direct memory access.

### Iterators
HybridArray provides STL-compatible proxy iterators:
* Dereferencing an iterator returns a proxy object.
* Enables use of standard algorithms (`std::for_each`, `std::transform`, etc.).
* Code reuse is achieved through CRTP-based iterator mixins.

### Manual Lifetime Management
To support non-trivial types in a SoA layout:
* Memory allocation is decoupled from object construction.
* Fields are constructed using placement new.
* Destructors are explicitly invoked when required.

Type traits (`std::is_trivially_destructible`) are used to eliminate unnecessary destruction loops for trivial types.

---

## Performance Model

HybridArray follows **Gustafson’s Law** rather than Amdahl’s Law:
* As problem size increases, data-parallel kernels dominate execution time.
* Initialization and abstraction overheads become negligible.
* SoA layouts scale efficiently with cache size and SIMD width.

The container is therefore optimized for large-scale, data-parallel workloads.

---

## When to Use HybridArray

### ✅ Recommended
* High Performance Computing
* Scientific simulations
* Data-parallel workloads
* AoS ↔ SoA benchmarking without refactoring
* Memory-layout experimentation

### ❌ Not Recommended
* Runtime-selectable memory layouts
* Pointer-stable element addresses
* Small, object-heavy applications

---

## Requirements

* C++17 or later
* Modern compiler (GCC, Clang, MSVC)
* No external dependencies

---

## Project Status

**🚧 Research / Thesis Project**

HybridArray is developed as part of an academic thesis focused on:
* Data-Oriented Design
* Compile-time metaprogramming
* Memory layout abstractions in modern C++

The project prioritizes architectural clarity and performance analysis over production hardening.

---

## License

MIT License (or specify your license here)

## Author

**Thomas Garrafa**
*Bachelor's Thesis — High Performance Computing / Computer Science*
