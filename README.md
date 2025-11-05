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

## 01. Repository Structure

### 01.01. Dynamic AoS/SoA System (/include folder)
This part is the core of the project, designed to allow flexible layout switching:

- **`OOpenCALStructs.h`** and   `OOpenCALArray.h`
  Implement the generic array abstraction supporting both AoS and SoA.  
  - Uses **X Macro (`Fields.h`)** to define fields.  
  - Uses **`Proxy`** object to transparently access elements regardless of layout.  
  - Manages memory allocation and aligned offsets for efficient access.  

- **`YourFieldsFileName.h`**  
  Lists all fields of the data structure, used by `_OOpenCALArray` via X Macro for automatic generation of getters, setters, and offsets.

- **`ComputeTime.h`**  
  Provides `computeTime` utility to measure **average execution time** of any function or lambda in milliseconds. Useful for benchmarking both AoS and SoA arrays.
 
 - **Single-proxy access  **`OOpenCALStructs_SingleProxy.h`** and   `OOpenCALArray_SingleProxy.h`**
  Used for testing only with their multiple stateless Proxy counterpart

- **Benchmark Examples**
  - `SoAoS_Classic.cpp`  
    Measures performance of AoS vs SoA layouts for a simple data structure with multiple fields.  
  - `SoAoS_Dynamic.cpp`  
    Uses the generic `OOpenCALArray` to benchmark dynamic AoS/SoA arrays with X Macro fields and proxy access.
    - `SoAoS_Dynamic_SingleProxy.cpp`  
    Uses the generic `OOpenCALArray_SingleProxy` to benchmark dynamic AoS/SoA arrays with X Macro fields and proxy access.
    
- **Tests**
	- Tested and implemented a serial dynamic SoA/AoS Sciddica cellular automata using two `OOpenCALArray`s 

---

### 01.02. Cellular Automaton: Sciddica

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

## 02. Usage
### 02.01. Defining your fields
Create a generic ```.h``` file and name it however you want. 
Inside of it, you have to define the proper fields your array have to contain. There are two commands you can use:
- ```Field(type, name)```: defines a field of type ```type```having name ```name```.  
- ```FieldArray(type, name, size)```: defines a C-Style static array of type ```type``` having name ```name``` and size ```size```.

For both ```Field``` and ```FieldArray```, you can add non-trivial datatypes, such as std::vector, std::string or user-defined classes.
In this last particular case, **it's user's responsability to implement an empty constructor, a destructor and an operator = for it** . However, the library won't work properly.

### 02.02. Including them in your code
This step is simple:
1. If there are non-trivial user-defined classes or standard libraries (e.g. std::vector, std::string) defined as type in your ```.h``` file, you have to define them (or include them)**before the step 2 below.** This is due how #include works. e.g:
```#include <vector>```
2. Include OOpenCALArray.h: ```#include "OOpenCALArray.h"```
3. Just create your ```OOpenCALArray``` arrays!
```OOpenCALArray myArray(10); //creates a dynamic SoA/AoS array having capacity=10```
4. You will magically have your **getters and setters**: you can access them using **operator[]**(just as a normal array!)
Let's say you have created this ```.h``` file:
```
Field(int, A)
Field(float, B)

FieldArray(float, GenericArrayName, 20)
```
then you can use your already defined getters and setters
```c++
int a = myArray[3].getA();
float b = 12.2;
myArray[5].setB(12);

float* genericFloatArr = myArray[1].getGenericArrayName();
myArray[2].getGFA()[4] = genericFloatArr[4];
```
**note: FieldArray fields don't have a setter function. Ideally, they should set a new pointer, but that's forbidden.**

#### What if I wanted to hardcode the layout of my arrays?
You can do it!
- ```_OOpenCALArray<soa> mySoAArray(10);``` for SoA 
- ```_OOpenCALArray<aos> myAoSArray(10);``` for AoS 

Notice that if you want to switch the internal structure dinamycally at **compile-time**, the first solution is what you are looking for.

### 02.03. Configuring Intellisense (VSCode)
**Note: you can skip this step if you don't want to disable Intellisense error squigglings or to enable autocompletition**
If you want to disable those annoying squiggling errors, or if you want your Intellisense to autocomplete your getters/setters, there are some few steps you have to follow:
#### In newer version of VSCode:
1.   File->Preferences->Settings->(Search right panel) c_cpp > Default: Defines.
Here, add:
- LAYOUT=soa (or aos, it doesn't change: here we're just configuring intellisense, not the project!)
- FIELDS_PATH=yourDotHFileName.h (the name of your file containing all your fields, without commas or other stuff)
2. File->Preferences->Settings->(Search right panel) c_cpp > Default: Include Path
Here, add:
- \${workspaceFolder}/include
- \${workspaceFolder}/

*\${workspaceFolder}* is a vsCode-defined variable that contains the absolute path of the folder you are working in (it's usually the folder that contains your already created .cpp file)
**It's essential that your**  ```.h``` **file is saved inside this working folder**
#### In older versions of VSCode
You have to modify your .json vsCode property file, adding these exact data 

### 02.04. Compile

Here's how to compile your code. There are some few flags you have to specify:
- -D LAYOUT=soa (or aos)
- -D FIELDS_PATH=yourDotHFileName.h
- -I ./folder (starting from your current one): this folder has to contain your ```.h``` file.
- -O3 for proper optimization
- (not necessary) -o yourOutputName if you want to change the default compiled fileName (default is a.out)

```bash
# For Structure of Arrays
g++ MyFileName.cpp -D FIELDS_PATH=yourDotFileName.h -D LAYOUT=soa -I ./ -o MyFileName_SoA.out -O3

# For Array of Structures
g++ MyFileName.cpp -D FIELDS_PATH=yourDotFileName.h -D LAYOUT=aos -I ./ -o MyFileName_AoS.out -O3
```

Run:
```bash
./MyFileName_AoS.out
#Or
./MyFileName_SoA.out
```

If you have hardcoded your array layouts (```_OOpenCALArray<soa> arr(12)```) simply don't add -D LAYOUT=soa (or AOS) during compilation

As it was said before, there are some stuff you can compile yourself:
- Sciddica.cpp: a dynamic SoA/Aos Sciddica cellular automata implementation
- SoAoS_Dynamic.cpp: a SoA/AoS benchmark you can compare with its "classic" counterpart (SoAoS_Classic)
- SoAoS_Dynamic_SingleProxy.cpp: a single-Proxy SoA/AoS benchmark you can compare with its "classic" counterpart (SoAoS_Classic)

## 03. Other stuff
You can run Sciddica without using this particular stucture. You will notice how the code complexity skyrockets with it:
```
### Sciddica Automaton
g++ sciddica.cpp -o sciddica.out
```
Layout selection is done at excecution time:
```
./sciddica.out --soa
./sciddica.out --aos
```
## 04. Notes

**The dynamic AoS/SoA system is the core innovation of the thesis.** It allows high-performance experiments without modifying the client code, using:
- X Macro for field definitions
- Proxy classe for layout-independent access
- Offset descriptors for SoA memory layout
- All benchmarks measure execution time using computeTime for fair comparison between layouts.

The Sciddica automaton demonstrates how the layout choice affects memory access patterns and performance in a real computational scenario.
