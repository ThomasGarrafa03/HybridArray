#include <stdio.h>
#include <cstdint>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>
#include <string>
#include <type_traits>
//pattern X Macro.

//Layout used 
enum Layout{
    SoA,
    AoS
};

template<Layout L>
class Proxy;
template<Layout L>
class _OOpenCALArray;

class OOpenCALCell;

//Field indexes, (size included)
enum FieldIndex{
    #define Field(type, name)\
        IDX_##name,
    #include "Fields.h"
    #undef Field
    Fields_Number //l'n-esimo elemento corrisponde al numero di campi che ho!
};

//Stores and computes AoS offsets and the memory total size
struct OffsetDescriptor{
    size_t offsets[Fields_Number];
    size_t totalSize;

    OffsetDescriptor(long capacity){
        size_t currentOffset = 0;

        #define Field(type, name) \
            /*Pattern used for proper memory alignment*/\
            currentOffset = (currentOffset + alignof(type) - 1) & ~(alignof(type) - 1);\
            offsets[IDX_##name] = currentOffset; \
            /*Given the previously computed (and aligned) offset, compute the next offset by multiplyng the capacity(number of elements) with the single element size and repeat, then align*/\
            currentOffset = currentOffset + (capacity * sizeof(type));
            
            #include "Fields.h"
        #undef Field

        totalSize = currentOffset;
    }
};

//generic AoS single cell data
class OOpenCALCell{
    private:
        #define Field(type, name) type name; 
        #include "Fields.h"
        #undef Field
    public:
        #undef Field
        #define Field(type, name) type get##name() {return name;}
        #include "Fields.h"
        #undef Field

        template<Layout L>
        friend class _OOpenCALArray;

        template<Layout L>
        friend class Proxy;
};

template <Layout L>
class Proxy{
    private:
        void *ptr; //the previously allocated raw data
        const OffsetDescriptor& descriptor; //the previously created descriptor reference
        size_t index; 
    public:
        Proxy(void* ptr, const OffsetDescriptor &descriptor): ptr(ptr), descriptor(descriptor) ,index(0) {}

        //getters and setters, based on the given Layout
        #define Field(type, name) \
            const type& get##name() const{ \
                if constexpr (L == AoS) { \
                    OOpenCALCell* base = (OOpenCALCell*)ptr;\
                    return base[index].name; \
                } else if constexpr (L == SoA){ \
                    void* base = ptr;\
                    type* field_ptr = (type*)((char*)base + descriptor.offsets[IDX_##name]);\
                    return field_ptr[index]; \
                }\
            }\
            \
            type& get##name(){ \
                if constexpr (L == AoS) { \
                    OOpenCALCell* base = (OOpenCALCell*)ptr;\
                    return base[index].name; \
                } else if constexpr (L == SoA){ \
                    void* base = ptr;\
                    type* field_ptr = (type*)((char*)base + descriptor.offsets[IDX_##name]);\
                    return field_ptr[index]; \
                }\
            }\
            \
            void set##name(const type &name) { \
                if constexpr(L == AoS) { \
                    OOpenCALCell* base = (OOpenCALCell*)ptr;\
                    base[index].name = name; \
                } else if constexpr(L == SoA){ \
                    void* base = ptr;\
                    type* field_ptr = (type*)((char*)base + descriptor.offsets[IDX_##name]);\
                    field_ptr[index] = name; \
                } \
            }
            #include "Fields.h"
        #undef Field

        friend class _OOpenCALArray<L>;
};

//The array with a predefined layout, defined in compile-time.
template<Layout L>
class _OOpenCALArray{
    private:
        Proxy<L> proxy; //a single proxy instance
        OffsetDescriptor descriptor; //offset description (used for SoA only)       
        long capacity; //the number of elements

    public: 
        _OOpenCALArray(long capacity): 
            capacity(capacity), 
            descriptor(capacity),
            proxy(nullptr, descriptor)
        {
            //create the proper pointer, given the layout.
            //notice that constexpr computes this branch during compile time
            if constexpr(L == AoS)
                proxy.ptr = new OOpenCALCell[capacity];
            else if constexpr (L == SoA){
                proxy.ptr = malloc(descriptor.totalSize);
                
                #define Field(type, name)\
                    /*Checks if data should be constructed manually: free() doesn't free dynamically allocated non-trivial data (e.g. vector, string, ...)*/\
                    /* This particular pattern is called placement new */\
                    /* It makes you construct data on a previously-allocated address.*/\
                    if constexpr(!std::is_trivially_destructible_v<type>){\
                        void* base = proxy.ptr;\
                        type* field_ptr = (type*)((char*)base + descriptor.offsets[IDX_##name]);\
                        for(int i = 0; i< capacity; i++){\
                            new (field_ptr + i) type(); /* Here I allocate data in the field_ptr + i address, calling the constructor (pattern is 'new (addr) constructor()' )*/\
                        }\
                    }
                    #include "Fields.h"
                #undef Field   
            }
        }

        //delete the pointer, given the layout
        ~_OOpenCALArray(){
            if constexpr(L == AoS)
                delete[] (OOpenCALCell*) proxy.ptr;
            else if constexpr(L == SoA){
                #define Field(type, name)\
                    /*Checks if data should be deleted manually: free() doesn't free dynamically allocated non-trivial data (e.g. vector, string, ...)*/\
                    if constexpr(!std::is_trivially_destructible_v<type>){\
                        void* base = proxy.ptr;\
                        type* field_ptr = (type*)((char*)base + descriptor.offsets[IDX_##name]);\
                        for(int i = 0; i< capacity; i++){\
                            using DestructorType = type; /* compiler cannot understand std::x. This using prevents it.*/\
                            field_ptr[i].~DestructorType();\
                        }\
                    }
                    #include "Fields.h"
                #undef Field   
                free(proxy.ptr);
            }
        }

        //set the proxy index and return a reference of it.
        Proxy<L> &operator[](size_t index){
            proxy.index = index;
            return proxy;
        }
};