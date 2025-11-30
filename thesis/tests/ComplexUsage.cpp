#include <iostream>
#include "../include/hybridArray/HybridArray_headers.h"

#define SIZE 10000

#define ARRAY_NAME Sciddica
#define INITFIELDS\
    Field(float, h)\
    Field(float, z)\
    FieldArray(float, f, 4)

#include "../include/hybridArray/HybridArray_core.h"

#define ARRAY_NAME Physics
#define INITFIELDS\
    Field(float, X)\
    Field(float, Y)\
    Field(float, Z)\
    Field(float, VX)\
    Field(float, VY)\
    Field(float, VZ)\

#include "../include/hybridArray/HybridArray_core.h"

int main(){
    PhysicsHybridArray myArr(1222);
    SciddicaHybridArray mySciddicaArr(123);
    myArr[4].setVX(12);

    mySciddicaArr[2].geth();

    //std::cout << arr1[3].getWeight()<<std::endl<<arr2[2].getClass()[1]<<std::endl;
}


