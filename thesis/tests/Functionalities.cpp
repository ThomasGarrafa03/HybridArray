#define INITFIELDS\
    Field(int, A)\
    Field(float, B)

#include "../include/hybridArray/HybridArray.h"

#include <iostream>

#define SIZE 10
#define SIZE2 20

int main(){
    /*SWAP*/
    HybridArray arr(SIZE);
    HybridArray toSwap(SIZE2);

    arr.swap(toSwap);

    /*STL-LIKE methods*/
    arr.at(12).setA(12);
    std::cout<< arr.capacity()<<std::endl;
    
    Proxy first = arr.front();
    Proxy last = arr.back();

    std::cout<<"First element (A)"<<arr.front().getA()<<std::endl<<"Last element (A)"<<arr.back().getA()<<std::endl;


    /*ITERATORS*/
    //forward
    int i = 0;
    for(Proxy p : arr){
        p.setA(i++);
    }

    auto b = arr.begin();
    while(b != arr.end()){
        std::cout<<(*b).getA()<<std::endl;
        ++b;
    }

    //reverse

    auto rb = arr.rbegin();
    while(rb != arr.rend()){
        std::cout<<"(reverse:)"<<(*rb).getA()<<std::endl;
        ++rb;
    }
}