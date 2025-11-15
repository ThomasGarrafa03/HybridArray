#include <iostream>
#include "../include/hybridArray/HybridArray_headers.h"

#define SIZE 10000

namespace Person{
    #define INITFIELDS \
        FieldArray(char, Name, 30)\
        Field(int, Age)\
        Field(float, Weight)

    #include "../include/hybridArray/HybridArray_core.h"

    #undef INITFIELDS
}

namespace School{
    #define INITFIELDS \
        FieldArray(char, Name, 30)\
        FieldArray(char, Class, 2)

    #include "../include/hybridArray/HybridArray_core.h"

    #undef INITFIELDS
}

int main(){
    Person::HybridArray arr1(SIZE);
    arr1[3].setWeight(12.2);

    School::HybridArray arr2(SIZE);
    arr2[2].getClass()[1] = '2';

    std::cout << arr1[3].getWeight()<<std::endl<<arr2[2].getClass()[1]<<std::endl;
}


