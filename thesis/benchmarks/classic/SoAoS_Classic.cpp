#include <cstdio>
#include <random>
#include <iomanip>
#include <string>
#include <iostream>
#include "../../utils/ComputeTime.h"

using namespace std;

#define SIZE 10000000
#define REPEAT_TIMES 10
#define ITERATIONS 10

struct SoA {
    float *x, *y, *z;
    float *vx, *vy, *vz;
    size_t _size;

    SoA(size_t size) : _size(size) {
        x = new float[size];
        y = new float[size];
        z = new float[size];
        vx = new float[size];
        vy = new float[size];
        vz = new float[size];
    }
    ~SoA() {
        delete[] x; delete[] y; delete[] z;
        delete[] vx; delete[] vy; delete[] vz;
    }
};

// --- AOS CLASSICO ---
struct ParticleAoS {
    float x, y, z;
    float vx, vy, vz;
};

class AoS {
    ParticleAoS* data;
public:
    AoS(size_t size) { data = new ParticleAoS[size]; }
    ~AoS() { delete[] data; }
    // Importante: inline per competere con la tua libreria
    inline ParticleAoS& operator[](size_t i) { return data[i]; }
};

float randomFloat() {
    static std::mt19937 gen(42);
    static std::uniform_real_distribution<float> dis(0.0f, 100.0f);
    return dis(gen);
}

int main() {
    SoA soa(SIZE);
    AoS aos(SIZE);
    float dt = 0.016f;

    auto simulation_soa = [&]() {
        // OTTIMIZZAZIONE MANUALE (Hoisting):
        // Copiamo i puntatori in variabili locali.
        // Questo dice al compilatore: "Non ricaricare soa.x dalla struct ogni volta!"
        // L'uso di __restrict (estensione GCC/Clang) sarebbe l'ideale qui.
        float* __restrict x = soa.x;
        float* __restrict y = soa.y;
        float* __restrict z = soa.z;
        float* __restrict vx = soa.vx;
        float* __restrict vy = soa.vy;
        float* __restrict vz = soa.vz;

        for (int iter = 0; iter < ITERATIONS; ++iter) {
            for (size_t i = 0; i < SIZE; ++i) {
                x[i] += vx[i] * dt;
                y[i] += vy[i] * dt;
                z[i] += vz[i] * dt;
            }
        }
    };

    auto simulation_aos = [&]() {
        for (int iter = 0; iter < ITERATIONS; ++iter) {
            for (size_t i = 0; i < SIZE; ++i) {
                ParticleAoS& cell = aos[i]; 

                cell.x += cell.vx * dt;
                cell.y += cell.vy * dt;
                cell.z += cell.vz * dt;
            }
        }
    };

    double soa_time = computeTime(simulation_soa, REPEAT_TIMES);
    double aos_time = computeTime(simulation_aos, REPEAT_TIMES); 

    printf("Classic SoA time: %f ms\n", soa_time);
    printf("Classic AoS time: %f ms\n", aos_time);
}