#include "ComputeTime.h"
#include <string.h>
#include <stdio.h>
#include <utility>
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

    auto aos_func = [&]() {
        for (int i = 0; i < SIZE; ++i) { aos.data[i].a = i; }
        for (int i = 0; i < SIZE; ++i) { aos.data[i].b = 0.5f * i; }
        for (int i = 0; i< SIZE; ++i) {aos.data[i].name = "Helo";}

        for (int i = 0; i < SIZE; ++i) { sumA += aos.data[i].a; }
        for (int i = 0; i < SIZE; ++i) { sumB += aos.data[i].b; }
        for (int i = 0; i< SIZE; ++i) {concat += aos.data[i].name;}
    };

    auto soa_func = [&]() {
        for (int i = 0; i < SIZE; ++i) { soa.a[i] = i; }
        for (int i = 0; i < SIZE; ++i) { soa.b[i] = 0.5f * i; }
        for (int i = 0; i< SIZE; ++i) {soa.name[i]= "Helo";}

        for (int i = 0; i < SIZE; ++i) { sumA += soa.a[i]; }
        for (int i = 0; i < SIZE; ++i) { sumB += soa.b[i]; }
        for (int i = 0; i< SIZE; ++i) {concat += soa.name[i];}

    };
    
    double time_aos = computeTime(aos_func, TIMES);
    double time_soa = computeTime(soa_func, TIMES);

    printf("AOS average time elapsed: %f\n", time_aos);
    printf("SOA average time elapsed: %f\n", time_soa);
}