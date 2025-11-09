#include "../../utils/ComputeTime.h"
#include <cstdio>
#include <string>

using namespace std;

#define SIZE 1000000
#define TIMES 100

struct SoA{
    int *a;
    float *b; 
    char *c;
    double *padding1;
    float *padding2;
    double *padding3;
    string *name;

    int size;

    SoA(int size){
        this->size = size;
        a = new int[size];
        b = new float[size]; 
        c = new char[size];
        padding1 = new double[size];
        padding2 = new float[size];
        padding3 = new double[size];
        name = new string[size];
    }

    ~SoA(){
        delete[]a;
        delete[]b;
        delete[]c;
        delete[]padding1;
        delete[]padding2;
        delete[]padding3;
        delete[] name;
    }
};

struct Data{
    int a;
    float b;
    char c;
    double padding1;
    float padding2;
    double padding3;
    string name;

    Data(){
        a = 0;
        b = 0.0;
        c = ' ';
    }
};

struct AoS{
    Data* data;
    int size;

    AoS(int size){
        this->size = size;
        data = new Data[size];
    }

    ~AoS(){
        delete[] data;
    }
};

int main() {
    SoA soa(SIZE);
    AoS aos(SIZE);

    double sumA = 0;
    double sumB = 0;
    string concat = "";

    auto aos_func_separated = [&]() {
        for (int i = 0; i < SIZE; ++i) { aos.data[i].a = i; }
        for (int i = 0; i < SIZE; ++i) { aos.data[i].b = 0.5f * i; }
        for (int i = 0; i< SIZE; ++i) {aos.data[i].name = "Helo";}

        for (int i = 0; i < SIZE; ++i) { sumA += aos.data[i].a; }
        for (int i = 0; i < SIZE; ++i) { sumB += aos.data[i].b; }
        for (int i = 0; i< SIZE; ++i) {concat += aos.data[i].name;}
    };

    auto soa_func_separated = [&]() {
        for (int i = 0; i < SIZE; ++i) { soa.a[i] = i; }
        for (int i = 0; i < SIZE; ++i) { soa.b[i] = 0.5f * i; }
        for (int i = 0; i< SIZE; ++i) {soa.name[i]= "Helo";}

        for (int i = 0; i < SIZE; ++i) { sumA += soa.a[i]; }
        for (int i = 0; i < SIZE; ++i) { sumB += soa.b[i]; }
        for (int i = 0; i< SIZE; ++i) {concat += soa.name[i];}

    };

    auto aos_func_single = [&]() {
        for (int i = 0; i < SIZE; ++i) { 
            aos.data[i].a = i; 
            aos.data[i].b = 0.5f * i; 
            aos.data[i].name = "Helo";
        }

        for (int i = 0; i < SIZE; ++i) { 
            sumA += aos.data[i].a; 
            sumB += aos.data[i].b;
            concat += aos.data[i].name; 
        }
    };

    auto soa_func_single = [&]() {
        for (int i = 0; i < SIZE; ++i) { 
            soa.a[i] = i; 
            soa.b[i] = 0.5f * i;
            soa.name[i]= "Helo";
        }

        for (int i = 0; i < SIZE; ++i) { 
            sumA += soa.a[i]; 
            sumB += soa.b[i];
            concat += soa.name[i];
        }
    };
    
    double time_aos_separated = computeTime(aos_func_separated, TIMES);
    double time_soa_separated = computeTime(soa_func_separated, TIMES);
    double time_aos_single = computeTime(aos_func_single, TIMES);
    double time_soa_single = computeTime(soa_func_single, TIMES);

    printf("AOS average time elapsed (separated for): %f\n", time_aos_separated);
    printf("SOA average time elapsed (separated for): %f\n", time_soa_separated);
    
    printf("AOS average time elapsed (single for): %f\n", time_aos_single);
    printf("SOA average time elapsed (single for): %f\n", time_soa_single);
}