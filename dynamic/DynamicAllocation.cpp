#include "OOpenCALStructs.h"
#include "ComputeTime.h"
using namespace std;

#define SIZE 10000000
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

    double sumA = 0;
    double sumB = 0;

    auto func = [&]() {
        for (int i = 0; i < SIZE; ++i) { arr[i].setA(i); }
        for (int i = 0; i < SIZE; ++i) { arr[i].setB(0.5f * i); }

        for (int i = 0; i < SIZE; ++i) { sumA += arr[i].getA(); }
        for (int i = 0; i < SIZE; ++i) { sumB += arr[i].getB(); }
    };

    double time = computeTime(func, TIMES); 
    
    printf("Average time elapsed: %f\n", time);

    #endif
}