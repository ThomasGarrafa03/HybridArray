//DEFINE NON-TRIVIAL TYPES BEFORE OOpenCALArray.h include (C++ standard)
#include <string>

#include "../../include/stateful/OOpenCALArray_SingleProxy.h"
#include "../../utils/ComputeTime.h"
using namespace std;

#define SIZE 1000000
#define TIMES 100

int main(int argc, char** argv){  
    
    OOpenCALArray arr(SIZE);
    
    int sumA = 0;
    float sumB = 0;
    string concat = "";

    //you can do this! arr[i].getIntArray()[j] = 19
    
    auto func = [&]() {
        for (int i = 0; i < SIZE; ++i) { arr[i].setA(i); }
        for (int i = 0; i < SIZE; ++i) { arr[i].setB(0.5f * i); }
        for (int i = 0; i< SIZE; ++i) {arr[i].setName("Helo");}

        for (int i = 0; i < SIZE; ++i) { sumA += arr[i].getA(); }
        for (int i = 0; i < SIZE; ++i) { sumB += arr[i].getB(); }
        for (int i = 0; i < SIZE; ++i) { concat += arr[i].getName(); }
    };

    double time = computeTime(func, TIMES); 
    
    printf("Average time elapsed: %f\n", time);
}