//DEFINE NON-TRIVIAL TYPES BEFORE OOpenCALArray.h include (C++ standard)
#include <string>

#define INITFIELDS\
    Field(int, A)\
    Field(float, B)\
    Field(std::string, Name)

#include "../../include/hybridArray/HybridArray.h"
#include "../../utils/ComputeTime.h"
using namespace std;

#define SIZE 10000000
#define TIMES 10

int main(int argc, char** argv){  
    
    HybridArray arr(SIZE);
    int sumA = 0;
    float sumB = 0;
    string concat = "";

    //you can do this! arr[i].getIntArray()[j] = 19
    
    auto func_separated = [&]() {
        for (int i = 0; i < SIZE; ++i) { arr[i].setA(i); }
        for (int i = 0; i < SIZE; ++i) { arr[i].setB(0.5f * i); }
        for (int i = 0; i< SIZE; ++i) {arr[i].setName("Helo");}

        for (int i = 0; i < SIZE; ++i) { sumA += arr[i].getA(); }
        for (int i = 0; i < SIZE; ++i) { sumB += arr[i].getB(); }
        for (int i = 0; i < SIZE; ++i) { concat += arr[i].getName(); }
    };

    auto func_single = [&]() {
        for (int i = 0; i < SIZE; ++i) { 
            arr[i].setA(i); 
            arr[i].setB(0.5f * i);
            arr[i].setName("Helo");
        }

        for (int i = 0; i < SIZE; ++i) { 
            sumA += arr[i].getA(); 
            sumB += arr[i].getB();
            concat += arr[i].getName();
        }
    };


    double time_separated = computeTime(func_separated, TIMES); 
    double time_single = computeTime(func_single, TIMES); 
    
    printf("Average time elapsed (separated for): %f\n", time_separated);
    printf("Average time elapsed (single for): %f\n", time_single);

}