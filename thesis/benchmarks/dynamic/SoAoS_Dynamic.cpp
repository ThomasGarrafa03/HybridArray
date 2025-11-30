#include <random>
#include <iomanip>
#include <string>
#include <iostream>

#define ARRAY_NAME Particles
#define INITFIELDS \
    Field(float, x) \
    Field(float, y) \
    Field(float, z) \
    Field(float, vx) \
    Field(float, vy) \
    Field(float, vz) \
    FieldArray(int, padding, 4) /* Useless data */

#include "../../include/hybridArray/HybridArray.h"
#include "../../utils/ComputeTime.h"
using namespace std;

#define SIZE 10000000
#define ITERATIONS 10
#define REPEAT_TIMES 10

float randomFloat() {
    static std::mt19937 gen(42); //fixed seed
    static std::uniform_real_distribution<float> dis(0.0f, 100.0f);
    return dis(gen);
}

int main(int argc, char** argv){  
    
    ParticlesHybridArray arr(SIZE);

    float dt = 0.016f;

    for(size_t i=0; i<SIZE; ++i) {
        arr[i].set_vx(randomFloat());
        arr[i].set_vy(randomFloat());
        arr[i].set_vz(randomFloat());
    }

    auto simulation = [&](){
        for (int iter = 0; iter < ITERATIONS; ++iter) {
            // Usiamo il tuo iteratore o accesso indice
            for (size_t i = 0; i < SIZE; ++i) {
                // L'accesso è identico per AoS e SoA grazie al Proxy!
                auto p = arr[i]; 
            
                float newX = p.get_x() + p.get_vx() * dt;
                float newY = p.get_y() + p.get_vy() * dt;
                float newZ = p.get_z() + p.get_vz() * dt;

                p.set_x(newX);
                p.set_y(newY);
                p.set_z(newZ);
            }
        }
    };

    double time = computeTime(simulation, REPEAT_TIMES);

    std::cout<< "Average "<< LAYOUT << " time elapsed: " << time <<"ms";

}