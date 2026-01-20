#ifndef SIZE 
#define SIZE 10000000
#endif

#define xSTR(a) STR(a)
#define STR(a) #a

#include <random>
#include <iomanip>
#include <string>
#include <iostream>

/******************/

//programmer defines its fields
#define INITFIELDS \
    Field(float, X) \
    Field(float, Y) \
    Field(float, Z) \
    Field(float, VX) \
    Field(float, VY) \
    Field(float, VZ) \

//includes HybridArray
#include "../../include/hybridArray/HybridArray.h"
/*******************/


#include "../../utils/ComputeTime.h"
using namespace std;

#define ITERATIONS 10
#define REPEAT_TIMES 10

float randomFloat() {
    static std::mt19937 gen(42); //fixed seed
    static std::uniform_real_distribution<float> dis(0.0f, 100.0f);
    return dis(gen);
}

int main(int argc, char** argv){  
    
    //done!
    HybridArray arr(SIZE);

    float dt = 0.016f;

    //array accessed via operator [], getters, setters
    for(size_t i=0; i<SIZE; ++i) {
        arr[i].setVX(randomFloat());
        arr[i].setVY(randomFloat());
        arr[i].setVZ(randomFloat());
    }

    auto simulation = [&](){
        for (int iter = 0; iter < ITERATIONS; ++iter) {
            for (size_t i = 0; i < SIZE; ++i) {
                auto p = arr[i]; 
            
                float newX = p.getX() + p.getVX() * dt;
                float newY = p.getY() + p.getVY() * dt;
                float newZ = p.getZ() + p.getVZ() * dt;

                p.setX(newX);
                p.setY(newY);
                p.setZ(newZ);
            }
        }
    };

    double time = computeTime(simulation, REPEAT_TIMES);


    std::cout<< "Dynamic " << xSTR(LAYOUT) <<" time elapsed: " << time <<" ms";
}