#include <stdio.h>
#include <cstdint>
#include <stdlib.h>
#include <string.h>

//pattern X Macro.

enum Layout{
    SoA,
    AoS
};

enum FieldIndex{
    #define Field(type, name)\
        IDX_##name,
    #include "DummyCell.h"
    #undef Field
    Fields_Number //l'n-esimo elemento corrisponde al numero di campi che ho!
};

Layout layout = SoA; //cambia layout qui!

class OOpenCALArray;

class OOpenCALCell{
    private:
        #define Field(type, name) type name;
        #include "DummyCell.h"
        #undef Field
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
        int size;
        
        size_t offsets[Fields_Number];

        void computeOffsets() {
            size_t currentOffset = 0;
            #define Field(type, name) \
                offsets[IDX_##name] = currentOffset; \
                /*Come funziona?*/ \
                currentOffset = (currentOffset + (size * sizeof(type))+ (alignof(double) - 1)) & ~(alignof(double) - 1); //todo aggiungi allineamento del tipo successivo, non del tipo stesso! (simil-type +1 (array?))

            #include "DummyCell.h"
            #undef Field
        }

    public: 
        OOpenCALArray(OOpenCALCell *ptr, int size){
            this->ptr = ptr;
            this->size = size;
            computeOffsets();
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
                            /*Prendo l'indirizzo iniziale della mia memoria allocata precedentemente*/ \
                            void* base = arr->ptr;\
                            \
                            /*Vi aggiungo l'offset precedentemente calcolato*/ \
                            void* aligned = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(base) + arr->offsets[IDX_##name]);\
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
                            \
                            /*Per semplicità (da rivedere) assegno sempre un offset di 8 celle.*/ \
                            void* aligned = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(base) + arr->offsets[IDX_##name]);\
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

int main(int argc, char** argv){
    const char *l = (argc > 1)? argv[1]: "";

    if(strcmp(l, "--soa") == 0)
        layout = SoA;
    else if(strcmp(l, "--aos") == 0)
        layout = AoS;
    else{ fprintf(stderr,"Parametro errato: specifica se eseguire attraverso SoA (soa) o AoS (aos)\n"); return 1;}

    //todo magari lavorare con il solo array renderebbe ancora più semplice la struttura (il distruttore distrugge il char*)
    OOpenCALCell* cells = new OOpenCALCell[10];
    OOpenCALArray arr(cells, 10);

    arr[0].setA(1231);
    arr[1].setA(2);
    arr[2].setB(12.32);

    arr[7].setA(19);
    arr[8].setA(10);
    arr[9].setA(103414);

    arr[5].setC('a');
    arr[9].setB(5.2);

    //todo fai in modo di poter iterare sui vari proxy in base al tipo
    for(int i = 0 ; i< 10; i++){
        printf("A[%d]: %d ; B[%d]: %f; C[%d]: %c\n", i, arr[i].getA(), i ,arr[i].getB(), i, arr[i].getC());
    }

    delete[]cells;
}