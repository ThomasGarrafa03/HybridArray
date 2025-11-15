#ifdef HYBRIDARRAY_HEADERS

#ifndef INITFIELDS
    #error "ERROR: INITFIELDS macro not included.\n USAGE:\n 1.define INITFIELDS, and inside it add your desired Field or FieldArray fields.\n 2. include this library"
#endif

#ifndef LAYOUT
    #error "ERROR: LAYOUT not specified: specify whether to compile (g++) in SoA (by adding -D LAYOUT = soa) or AoS (by adding -D LAYOUT = aos).")
#endif

template<Layout L>
class Proxy;
template<Layout L>
class StaticArray;
template<Layout L>
class Iterator;
template<Layout L>
class ConstIterator;
template<Layout L>
class ReverseIterator;
template<Layout L>
class ConstReverseIterator;

class AoSCell;

/*Field indexes, (size included)*/
enum FieldIndex{
    #define Field(type, name)\
        IDX_##name,

    #define FieldArray(type, name, size)\
        IDX_##name,

    INITFIELDS
    #undef Field
    #undef FieldArray
    Fields_Number //n-th enum saves the defined fields number
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
        
        INITFIELDS
        #undef Field
        #undef FieldArray

        totalSize = currentOffset;
    }
};

//generic AoS single cell data
class AoSCell{
    private:
        #define Field(type, name) type name; 
        #define FieldArray(type, name, size) type name[size];

        INITFIELDS
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

        INITFIELDS
        #undef Field
        #undef FieldArray

        template<Layout L>
        friend class StaticArray;

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
                    AoSCell* base = static_cast<AoSCell*>(ptr);\
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
                    AoSCell* base = static_cast<AoSCell*>(ptr);\
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
                    AoSCell* base = static_cast<AoSCell*>(ptr);\
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
                    AoSCell* base = static_cast<AoSCell*>(ptr);\
                    return base[index].name; \
                } else if constexpr (L == soa){ \
                    void* base = ptr;\
                    type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]);\
                    return field_ptr + (index*size); \
                }\
            }\
            const type* get##name() const{\
                if constexpr (L == aos) { \
                    AoSCell* base = static_cast<AoSCell*>(ptr);\
                    return base[index].name; \
                } else if constexpr (L == soa){ \
                    void* base = ptr;\
                    type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]);\
                    return field_ptr + (index*size); \
                }\
            }

        INITFIELDS
        #undef Field
        #undef FieldArray

        friend class StaticArray<L>;
};

//The array with a predefined layout, defined in compile-time.
template<Layout L>
class StaticArray{
    private:
        void * ptr;
        OffsetDescriptor descriptor; //offset description (used for SoA only)       
        size_t capacity; //the number of elements

    public: 
        StaticArray(size_t capacity): 
            capacity(capacity), 
            descriptor(capacity),
            ptr(nullptr)
        {
            //create the proper pointer, given the layout.
            //notice that constexpr computes this branch during compile time
            if constexpr(L == aos)
                ptr = new AoSCell[capacity];
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
                
                INITFIELDS
                #undef Field   
                #undef FieldArray
            }
        }

        // todo ADD COPY CONSTRUCTOR AND OPERATOR =.
        StaticArray(const StaticArray& other) = delete;
        StaticArray& operator=(const StaticArray& other) = delete;

        //delete the pointer, given the layout
        ~StaticArray(){
            if constexpr(L == aos)
                delete[] static_cast<AoSCell*>(ptr);
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
                INITFIELDS
                #undef Field   
                #undef FieldArray

                free(ptr);
            }
        }

        /*ITERATORS*/
        //classic forward iterators
        Iterator begin(){
            return Iterator(*this, 0);
        }

        Iterator end(){
            return Iterator(*this, capacity);
        }

        const ConstIterator cbegin() const{
            return ConstIterator(*this, 0);
        }

        const ConstIterator cend() const{
            return ConstIterator(*this, capacity);
        }

        //reverse iterators
        ReverseIterator rbegin(){
            return ReverseIterator(*this, capacity-1);
        }

        ReverseIterator rend(){
            return ReverseIterator(*this, (size_t)-1);
        }

        const ConstReverseIterator crbegin() const{
            return ConstReverseIterator(*this, capacity -1);
        }

        const ConstReverseIterator crend() const{
            return ConstReverseIterator(*this, (size_t)-1);
        }

        /*CAPACITY*/
        size_t capacity() const{
            return capacity;
        }

        /*ELEMENT ACCESS*/
        Proxy<L> operator[](size_t index){
            return Proxy<L>(ptr, descriptor, index);
        } 

        Proxy<L> at(size_t index){
            if(index >= capacity) 
                throw std::out_of_range("Index specified needs to be less than capacity.");
            return Proxy<L>(ptr, descriptor, index);
        } 

        Proxy<L> front(){
            return Proxy<L>(ptr, descriptor, 0);
        } 

        Proxy<L> back(){
            return Proxy<L>(ptr, descriptor, capacity-1);
        } 

        /*MODIFIERS*/
        void swap (StaticArray &other){
            std::swap(ptr, other.ptr);
            std::swap(capacity, other.capacity);
            std::swap(descriptor, other.descriptor);
        }

        friend class Iterator;
        friend class ConstIterator;
        friend class ReverseIterator;
        friend class ConstReverseIterator;
};

template<Layout L>
class Iterator{
    private:
        StaticArray<L> & staticArray;
        size_t index;
    public: 
        Iterator(StaticArray<L> & staticArray, size_t index): staticArray(staticArray), index(index){}

        Iterator& operator++(){
            index ++;
            return *this;
        }

        Iterator& operator--(){
            index --;
            return *this;
        }

        //todo add checks!
        Iterator& operator-=(size_t n){
            index -= n;
            return *this;
        }

        Iterator& operator+=(size_t n){
            index += n;
            return *this;
        }

        Proxy<L> operator*(){
            return Proxy<L>(staticArray.ptr, staticArray.descriptor, index);
        }

        bool operator==(Iterator& other){
            return index == other.index;
        }
};

template<Layout L>
class ReverseIterator{
    private:
        StaticArray<L> & staticArray;
        size_t index;
    public: 
        ReverseIterator(StaticArray<L> & staticArray, size_t index): staticArray(staticArray), index(index){}

        ReverseIterator& operator++(){
            index --;
            return *this;
        }

        ReverseIterator& operator--(){
            index ++;
            return *this;
        }

        //todo add checks!
        ReverseIterator& operator-=(size_t n){
            index += n;
            return *this;
        }

        ReverseIterator& operator+=(size_t n){
            index -= n;
            return *this;
        }

        Proxy<L> operator*(){
            return Proxy<L>(staticArray.ptr, staticArray.descriptor, index);
        }

        bool operator==(ReverseIterator& other){
            return index == other.index;
        }
};

template<Layout L>
class ConstIterator{
    private:
        const StaticArray<L> & staticArray;
        size_t index;
    public: 
        ConstIterator(const StaticArray<L> & staticArray, size_t index): staticArray(staticArray), index(index){}

        const Proxy<L> operator*()const {
            reutrn Proxy<L>(staticArray.ptr, staticArray.descriptor, index);
        }

        bool operator==(Iterator& other){
            return index == other.index;
        }
};

template<Layout L>
class ConstReverseIterator{
    private:
        const StaticArray<L> & staticArray;
        size_t index;
    public: 
        ConstReverseIterator(const StaticArray<L> & staticArray, size_t index): staticArray(staticArray), index(index){}

        const Proxy<L> operator*()const {
            return Proxy<L>(staticArray.ptr, staticArray.descriptor, index);
        }

        bool operator==(Iterator& other){
            return index == other.index;
        }
};

using HybridArray = StaticArray<LAYOUT>;

#endif