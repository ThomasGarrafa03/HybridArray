#include <stdio.h>
#include <cstdint>
#include <stdlib.h>

//pattern X Macro.
#define Field(type, name) type name;

enum Layout{
    SoA,
    AoS
};

Layout layout = AoS; //cambia layout qui!

class OOpenCALArray;

class OOpenCALCell{
    private:
        #include "DummyCell.h"
    public:
        #undef Field
        #define Field(type, name) const type& get##name() const{return name;}
        #include "DummyCell.h"
        #undef Field

        void *operator new[](size_t size){
            //if(layout == AoS) 
            
            return malloc(size+sizeof(double));
            
            //dovrei allocare idealmente meno memoria (idealmente, si ha meno offset in SoA)
            //else if(layout == SoA)...
        }

        void operator delete[](void* ptr){
            free(ptr);
        }

        friend class OOpenCALArray;
};


class OOpenCALArray{
    private:
        void *ptr;
    public: 
        OOpenCALArray(OOpenCALCell *ptr){
            this->ptr = ptr;
        }

        class Proxy{
            private:
                OOpenCALArray*arr;
                size_t index;
            public:
                Proxy(OOpenCALArray* arr, size_t index): arr(arr), index(index) {}

                #define Field(type, name) \
                    const type& get##name() const{ \
                        if (layout == AoS) { \
                            OOpenCALCell* base = static_cast<OOpenCALCell*>(arr->ptr); \
                            return base[index].name; \
                        } else { \
                            /* SoA: campi separati */ \
                            /*devo allineare la memoria, aggiungendo un offset.*/\
                            /*Per semplicità (da rivedere) assegno sempre un offset di 8 celle.*/ \
                            void* base = arr->ptr;\
                            void* aligned = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(base) + sizeof(double) - 1) & ~(sizeof(double) - 1));\
                            type* field_ptr = static_cast<type*>(aligned);\
                            return field_ptr[index]; \
                        }\
                    }\
                    void set##name(type name) { \
                        if (layout == AoS) { \
                            OOpenCALCell* base = static_cast<OOpenCALCell*>(arr->ptr); \
                            base[index].name = name; \
                        } else { \
                            /* SoA: campi separati */ \
                            /*devo allineare la memoria, aggiungendo un offset.*/\
                            /*Per semplicità (da rivedere) assegno sempre un offset di 8 celle.*/ \
                            void* base = arr->ptr;\
                            void* aligned = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(base) + sizeof(double) - 1) & ~(sizeof(double) - 1));\
                            type* field_ptr = static_cast<type*>(aligned);\
                            field_ptr[index] = name; \
                        } \
                    }
            #include "DummyCell.h"
            #undef Field

            friend class OOpenCALArray;
        };

        Proxy operator[](size_t index){
            return Proxy{this, index};
        }
};

int main(){
    OOpenCALCell* cells = new OOpenCALCell[10];
    OOpenCALArray arr(cells);
    arr[7].setA(19);
    arr[9].setB(5.2);

    printf("%d\n%f", arr[7].getA(), arr[9].getB());

    delete[]cells;
}