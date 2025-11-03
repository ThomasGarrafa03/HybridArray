#include "OOpenCALStructs.h"
#include "ComputeTime.h"
using namespace std;

#define SIZE 100
#define TIMES 100

//Direct soa/aos compilation
//defined(x) checks if x is either specified or not
#if defined(soa)
    using OOpenCALArray = _OOpenCALArray<SoA>;
#elif defined(aos)
    using OOpenCALArray = _OOpenCALArray<AoS>;
#else
    #error "Parametro errato o non specificato: specifica se compilare (g++) in SoA (aggiungendo -D soa) o AoS (aggiungendo -D aos).");
#endif

int main(int argc, char** argv){  
    #if defined(soa) || defined(aos)
    OOpenCALArray arr(SIZE); 

    int sumA = 0;
    float sumB = 0;
    string concat = "";
    long long sumArr = 0;

    for (int i = 0; i < SIZE; ++i) { arr[i].setA(i); }
    for (int i = 0; i < SIZE; ++i) { arr[i].setB(0.5f * i); }
    for (int i = 0; i< SIZE; ++i) {arr[i].setC('a' + (i % 26));}
    for (int i = 0; i< SIZE; ++i) {arr[i].setName(to_string(i));}
    for(int i = 0; i< SIZE; ++i){
        for(int j = 0; j < 15; ++j) {
            arr[i].getIntArr()[j] = i + j;
        }
    }
        
    for (int i = 0; i < SIZE; ++i) { sumA += arr[i].getA(); }
    for (int i = 0; i < SIZE; ++i) { sumB += arr[i].getB(); }
    for (int i = 0; i < SIZE; ++i) { concat += arr[i].getC(); }
    for (int i = 0; i < SIZE; ++i) { concat += arr[i].getName();}
    for(int i = 0; i< SIZE; ++i){
        for(int j = 0; j < 15; ++j) {
            sumArr = arr[i].getIntArr()[j];
        }
    }
    
    printf("Sum A: %d\nSum B: %f\nConcat: %s\nsum Arr: %lld\n", sumA, sumB, concat.c_str(), sumArr);

    #endif
}