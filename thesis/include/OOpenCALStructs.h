#ifndef OOCAL_STRUCTS
#define OOCAL_STRUCTS

//"If you want to stringify the result of expansion of a macro argument, you have to use two levels of macros." (see compiler docs)
#define QUOTE(x) #x
#define STR(x) QUOTE(x)

#ifndef FIELDS_PATH
    #define FIELDS_PATH Fields.h
#endif

#include <cstdlib> //for malloc, free, ...
#include <type_traits> //for is_trivially_destructible_v<V>
#include <new> //for placement new

//pattern X Macro.

//Layout used 
enum Layout{
    soa,
    aos
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

    #define FieldArray(type, name, size)\
        IDX_##name,

    #include STR(FIELDS_PATH)
    #undef Field
    #undef FieldArray
    Fields_Number //l'n-esimo elemento corrisponde al numero di campi che ho!
};

//Stores and computes AoS offsets and the memory total size
struct OffsetDescriptor{
    size_t offsets[Fields_Number];
    size_t totalSize;

    OffsetDescriptor(size_t capacity){
        size_t currentOffset = 0;

        #define Field(type, name) \
            /*Pattern used for proper memory alignment*/\
            currentOffset = (currentOffset + alignof(type) - 1) & ~(alignof(type) - 1);\
            offsets[IDX_##name] = currentOffset; \
            /*Given the previously computed (and aligned) offset, compute the next offset by multiplyng the capacity(number of elements) with the single element size and repeat, then align*/\
            currentOffset = currentOffset + (capacity * sizeof(type));
            
        #define FieldArray(type, name, size)\
            currentOffset = (currentOffset + alignof(type) - 1) & ~(alignof(type) - 1);\
            offsets[IDX_##name] = currentOffset; \
            /*Given the previously computed (and aligned) offset, compute the next offset by multiplyng the capacity(number of elements) with the single element size and repeat, then align*/\
            currentOffset = currentOffset + (capacity * sizeof(type)*size);
        
        #include STR(FIELDS_PATH)
        #undef Field
        #undef FieldArray

        totalSize = currentOffset;
    }
};

//generic AoS single cell data
class OOpenCALCell{
    private:
        #define Field(type, name) type name; 
        #define FieldArray(type, name, size) type name[size];

        #include STR(FIELDS_PATH)
        #undef Field
        #undef FieldArray
    public:
        #define Field(type, name)\
            type& get##name() {\
                return name;\
            }\
            \
            const type& get##name() const {\
                return name;\
            }\
            \
            void set##name(const type& name){\
                this->name = name;\
            }

        #define FieldArray(type, name, size)\
            type* get##name(){return name;}\
            const type* get##name() const{return name;}

        #include STR(FIELDS_PATH)
        #undef Field
        #undef FieldArray

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
        Proxy(void* ptr, const OffsetDescriptor &descriptor, size_t index): ptr(ptr), descriptor(descriptor) ,index(index) {}

        //getters and setters, based on the given Layout
        #define Field(type, name) \
            const type& get##name() const{ \
                if constexpr (L == aos) { \
                    OOpenCALCell* base = static_cast<OOpenCALCell*>(ptr);\
                    return base[index].name; \
                } else if constexpr (L == soa){ \
                    void* base = ptr;\
                    type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]);\
                    return field_ptr[index]; \
                }\
            }\
            \
            type& get##name(){ \
                if constexpr (L == aos) { \
                    OOpenCALCell* base = static_cast<OOpenCALCell*>(ptr);\
                    return base[index].name; \
                } else if constexpr (L == soa){ \
                    void* base = ptr;\
                    type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]);\
                    return field_ptr[index]; \
                }\
            }\
            \
            void set##name(const type &name) { \
                if constexpr(L == aos) { \
                    OOpenCALCell* base = static_cast<OOpenCALCell*>(ptr);\
                    base[index].name = name; \
                } else if constexpr(L == soa){ \
                    void* base = ptr;\
                    type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]);\
                    field_ptr[index] = name; \
                } \
            }
        
        //static c-style arrays    
        #define FieldArray(type, name, size)\
            type* get##name(){\
                if constexpr (L == aos) { \
                    OOpenCALCell* base = static_cast<OOpenCALCell*>(ptr);\
                    return base[index].name; \
                } else if constexpr (L == soa){ \
                    void* base = ptr;\
                    type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]);\
                    return field_ptr + (index*size); \
                }\
            }\
            const type* get##name() const{\
                if constexpr (L == aos) { \
                    OOpenCALCell* base = static_cast<OOpenCALCell*>(ptr);\
                    return base[index].name; \
                } else if constexpr (L == soa){ \
                    void* base = ptr;\
                    type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]);\
                    return field_ptr + (index*size); \
                }\
            }

        #include STR(FIELDS_PATH)
        #undef Field
        #undef FieldArray

        friend class _OOpenCALArray<L>;
};

//The array with a predefined layout, defined in compile-time.
template<Layout L>
class _OOpenCALArray{
    private:
        void * ptr;
        OffsetDescriptor descriptor; //offset description (used for SoA only)       
        size_t capacity; //the number of elements

    public: 
        _OOpenCALArray(size_t capacity): 
            capacity(capacity), 
            descriptor(capacity),
            ptr(nullptr)
        {
            //create the proper pointer, given the layout.
            //notice that constexpr computes this branch during compile time
            if constexpr(L == aos)
                ptr = new OOpenCALCell[capacity];
            else if constexpr (L == soa){
                ptr = malloc(descriptor.totalSize);
                
                #define Field(type, name)\
                    /*Checks if data should be constructed manually: free() doesn't free dynamically allocated non-trivial data (e.g. vector, string, ...)*/\
                    /* This particular pattern is called placement new */\
                    /* It makes you construct data on a previously-allocated address.*/\
                    if constexpr(!std::is_trivially_destructible_v<type>){\
                        void* base = ptr;\
                        type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]);\
                        for(int i = 0; i< capacity; i++){\
                            new (field_ptr + i) type(); /* Here I allocate data in the field_ptr + i address, calling the constructor (pattern is 'new (addr) constructor()' )*/\
                        }\
                    }
                
                //maybe superfluous, but complete. Now you can create, for example, a fixed array of string (string names[30])
                #define FieldArray(type, name, size)\
                    if constexpr(!std::is_trivially_destructible_v<type>){\
                        void* base = ptr;\
                        type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]);\
                        for(int i = 0; i< capacity*size; i++){\
                            new (field_ptr + i) type(); /* Here I allocate data in the field_ptr + i address, calling the constructor (pattern is 'new (addr) constructor()' )*/\
                        }\
                    }
                
                #include STR(FIELDS_PATH)
                #undef Field   
                #undef FieldArray
            }
        }

        //delete the pointer, given the layout
        ~_OOpenCALArray(){
            if constexpr(L == aos)
                delete[] static_cast<OOpenCALCell*>(ptr);
            else if constexpr(L == soa){
                #define Field(type, name)\
                    /*Checks if data should be deleted manually: free() doesn't free dynamically allocated non-trivial data (e.g. vector, string, ...)*/\
                    if constexpr(!std::is_trivially_destructible_v<type>){\
                        void* base = ptr;\
                        type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]);\
                        for(int i = 0; i< capacity; i++){\
                            using DestructorType = type; /* compiler cannot understand std::x. This using prevents it.*/\
                            field_ptr[i].~DestructorType();\
                        }\
                    }
                
                #define FieldArray(type, name, size)\
                    if constexpr(!std::is_trivially_destructible_v<type>){\
                        void* base = ptr;\
                        type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]);\
                        for(int i = 0; i< capacity*size; i++){\
                            using DestructorType = type; /* compiler cannot understand std::x. This using prevents it.*/\
                            field_ptr[i].~DestructorType();\
                        }\
                    }    
                #include STR(FIELDS_PATH)
                #undef Field   
                #undef FieldArray

                free(ptr);
            }
        }

        Proxy<L> operator[](size_t index){
            return Proxy<L>(ptr, descriptor, index);
        } 
};

#endif