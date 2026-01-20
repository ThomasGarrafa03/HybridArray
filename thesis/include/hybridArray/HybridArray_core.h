#if defined(HYBRIDARRAY_HEADERS) && defined(INITFIELDS) && defined(LAYOUT)

#ifndef ARRAY_NAME
    #define ARRAY_NAME 
#endif

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define CURRENT_PROXY CONCAT(ARRAY_NAME, Proxy)
#define CURRENT_STATICARRAY CONCAT(ARRAY_NAME, StaticArray)
#define CURRENT_HYBRIDARRAY CONCAT(ARRAY_NAME, HybridArray)
#define CURRENT_OFFSETDESCRIPTOR CONCAT(ARRAY_NAME,OffsetDescriptor)
#define CURRENT_FIELDINDEX CONCAT(ARRAY_NAME,FieldIndex)
#define CURRENT_AOSCELL CONCAT(ARRAY_NAME,AoSCell)


template<Layout L>
class CURRENT_PROXY;
template<Layout L>
class CURRENT_STATICARRAY;

/*Field _indexes, (size included)*/
enum CURRENT_FIELDINDEX {
	#define Field(type, name) \
		IDX_##name,

	#define FieldArray(type, name, size) \
		IDX_##name,

	INITFIELDS
	#undef Field
	#undef FieldArray
	Fields_Number // n-th enum saves the defined fields number
};

// Stores and computes AoS offsets and the memory total size
struct CURRENT_OFFSETDESCRIPTOR{
	size_t offsets[Fields_Number] = {};
	size_t totalSize = 0;

	constexpr CURRENT_OFFSETDESCRIPTOR(size_t _capacity) {
		size_t currentOffset = 0;

		#define Field(type, name) \
			currentOffset = (currentOffset + alignof(type) - 1) & ~(alignof(type) - 1); \
			offsets[IDX_##name] = currentOffset; \
			currentOffset = currentOffset + (_capacity * sizeof(type));

		#define FieldArray(type, name, size) \
			currentOffset = (currentOffset + alignof(type) - 1) & ~(alignof(type) - 1); \
			offsets[IDX_##name] = currentOffset; \
			currentOffset = currentOffset + (_capacity * sizeof(type) * size);

		INITFIELDS
		#undef Field
		#undef FieldArray

		totalSize = currentOffset;
	}
};

// Generic AoS single cell data
class CURRENT_AOSCELL {
private:
	#define Field(type, name) type name;
	#define FieldArray(type, name, size) type name[size];

	INITFIELDS

	#undef Field
	#undef FieldArray

	friend class CURRENT_PROXY<aos>;
};

template <Layout L>
class CURRENT_PROXY {
private:
	void* ptr; //pointer to the raw memory block
	const CURRENT_OFFSETDESCRIPTOR& descriptor; // reference to the layout metadata
	size_t _index; //logical index of the element

public:
	CURRENT_PROXY(void* ptr, const CURRENT_OFFSETDESCRIPTOR &descriptor, size_t _index)
		: ptr(ptr), descriptor(descriptor), _index(_index) {}

	// Getters and setters
	#define Field(type, name) \
		const type& get##name() const { \
			if constexpr (L == aos) { \
				CURRENT_AOSCELL* base = static_cast<CURRENT_AOSCELL*>(ptr); \
				return base[_index].name; \
			} else { \
				type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(ptr) + descriptor.offsets[IDX_##name]); \
				return field_ptr[_index]; \
			} \
		} \
		type& get##name() { \
			if constexpr (L == aos) { \
				CURRENT_AOSCELL* base = static_cast<CURRENT_AOSCELL*>(ptr); \
				return base[_index].name; \
			} else { \
				type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(ptr) + descriptor.offsets[IDX_##name]); \
				return field_ptr[_index]; \
			} \
		} \
		void set##name(const type &name) { \
			if constexpr (L == aos) { \
				CURRENT_AOSCELL* base = static_cast<CURRENT_AOSCELL*>(ptr); \
				base[_index].name = name; \
			} else { \
				type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(ptr) + descriptor.offsets[IDX_##name]); \
				field_ptr[_index] = name; \
			} \
		}

	#define FieldArray(type, name, size) \
		type* get##name() { \
			if constexpr (L == aos) { \
				CURRENT_AOSCELL* base = static_cast<CURRENT_AOSCELL*>(ptr); \
				return base[_index].name; \
			} else { \
				type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(ptr) + descriptor.offsets[IDX_##name]); \
				return field_ptr + (_index * size); \
			} \
		} \
		const type* get##name() const { \
			if constexpr (L == aos) { \
				CURRENT_AOSCELL* base = static_cast<CURRENT_AOSCELL*>(ptr); \
				return base[_index].name; \
			} else { \
				type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(ptr) + descriptor.offsets[IDX_##name]); \
				return field_ptr + (_index * size); \
			} \
		}

	INITFIELDS
	#undef Field
	#undef FieldArray
    
    size_t index() const{return _index;}

	friend class CURRENT_STATICARRAY<L>;
};

template<Layout L>
class CURRENT_STATICARRAY {
private:
	void* ptr;
	CURRENT_OFFSETDESCRIPTOR descriptor;
	size_t _capacity;

	// CRTP-based iterator base
	template<typename Derived, typename ArrayType, typename ProxyType>
	class base_iterator {
	protected:
		ArrayType* staticArray;
		size_t _index;

	public:
		using difference_type   = std::ptrdiff_t;
		using value_type        = ProxyType;
		using pointer           = ProxyType*;
		using reference         = ProxyType;
		using iterator_category = std::random_access_iterator_tag;

		base_iterator(ArrayType* arr, size_t idx) : staticArray(arr), _index(idx) {}

		Derived& operator+=(difference_type n) { _index += n; return static_cast<Derived&>(*this); }
		Derived& operator-=(difference_type n) { _index -= n; return static_cast<Derived&>(*this); }
		Derived& operator++() { ++this->_index; return static_cast<Derived&>(*this); }
        Derived operator++(int) { Derived tmp = static_cast<Derived&>(*this); ++(*this); return tmp; }
        Derived& operator--() { --this->_index; return static_cast<Derived&>(*this); }
        Derived operator--(int) { Derived tmp = static_cast<Derived&>(*this); --(*this); return tmp; }

		Derived operator+(difference_type n) const { return Derived(staticArray, _index + n); }
		Derived operator-(difference_type n) const { return Derived(staticArray, _index - n); }
		difference_type operator-(const Derived& other) const { return _index - other._index; }
		
		reference operator[](difference_type n) const { return *(*static_cast<const Derived*>(this) + n); }
		reference operator*() const { return ProxyType(staticArray->ptr, staticArray->descriptor, _index); }

		bool operator==(const Derived& other) const { return _index == other._index; }
		bool operator!=(const Derived& other) const { return _index != other._index; }
		bool operator<(const Derived& other) const { return _index < other._index; }
		bool operator>(const Derived& other) const { return _index > other._index; }
		bool operator<=(const Derived& other) const { return _index <= other._index; }
		bool operator>=(const Derived& other) const { return _index >= other._index; }
	};

	class iterator : public base_iterator<iterator, CURRENT_STATICARRAY<L>, CURRENT_PROXY<L>> {
		using base = base_iterator<iterator, CURRENT_STATICARRAY<L>, CURRENT_PROXY<L>>;
	public:
		iterator(CURRENT_STATICARRAY<L>& arr, size_t idx) : base(&arr, idx) {}
		friend class const_iterator;
	};

	class const_iterator : public base_iterator<const_iterator, const CURRENT_STATICARRAY<L>, const CURRENT_PROXY<L>> {
		using base = base_iterator<const_iterator, const CURRENT_STATICARRAY<L>, const CURRENT_PROXY<L>>;
	public:
		const_iterator(const CURRENT_STATICARRAY<L>& arr, size_t idx) : base(&arr, idx) {}
		const_iterator(const iterator& it) : base(it.staticArray, it._index) {}
	};
	
    using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
	CURRENT_STATICARRAY(size_t _capacity) :
		_capacity(_capacity),
		descriptor(_capacity),
		ptr(nullptr)
	{
		if constexpr(L == aos)
			ptr = new CURRENT_AOSCELL[_capacity];
		else if constexpr(L == soa) {
			ptr = malloc(descriptor.totalSize);

			#define Field(type, name) \
				if constexpr(!std::is_trivially_destructible_v<type>) { \
					void* base = ptr; \
					type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]); \
					for(int i = 0; i < _capacity; i++) { \
						new (field_ptr + i) type(); \
					} \
				}

			#define FieldArray(type, name, size) \
				if constexpr(!std::is_trivially_destructible_v<type>) { \
					void* base = ptr; \
					type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]); \
					for(int i = 0; i < _capacity*size; i++) { \
						new (field_ptr + i) type(); \
					} \
				}

			INITFIELDS
			#undef Field
			#undef FieldArray
		}
	}

	CURRENT_STATICARRAY(const CURRENT_STATICARRAY& other) = delete;
	CURRENT_STATICARRAY& operator=(const CURRENT_STATICARRAY& other) = delete;

	~CURRENT_STATICARRAY() {
		if constexpr(L == aos)
			delete[] static_cast<CURRENT_AOSCELL*>(ptr);
		else if constexpr(L == soa) {
			#define Field(type, name) \
				if constexpr(!std::is_trivially_destructible_v<type>) { \
					void* base = ptr; \
					type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]); \
					for(int i = 0; i < _capacity; i++) { \
						using DestructorType = type; \
						field_ptr[i].~DestructorType(); \
					} \
				}

			#define FieldArray(type, name, size) \
				if constexpr(!std::is_trivially_destructible_v<type>) { \
					void* base = ptr; \
					type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]); \
					for(int i = 0; i < _capacity*size; i++) { \
						using DestructorType = type; \
						field_ptr[i].~DestructorType(); \
					} \
				}

			INITFIELDS
			#undef Field
			#undef FieldArray

			free(ptr);
		}
	}

	iterator begin() { return iterator(*this, 0); }
	iterator end() { return iterator(*this, _capacity); }
	const const_iterator cbegin() const { return const_iterator(*this, 0); }
	const const_iterator cend() const { return const_iterator(*this, _capacity); }
	reverse_iterator rbegin() { return reverse_iterator(end()); }
	reverse_iterator rend() { return reverse_iterator(begin()); }
	const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
	const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

	size_t capacity() const { return _capacity; }

	CURRENT_PROXY<L> operator[](size_t _index) { return CURRENT_PROXY<L>(ptr, descriptor, _index); }
	CURRENT_PROXY<L> at(size_t _index) {
		if(_index >= _capacity)
			throw std::out_of_range("Index specified needs to be less than capacity.");
		return CURRENT_PROXY<L>(ptr, descriptor, _index);
	}
	CURRENT_PROXY<L> front() { return CURRENT_PROXY<L>(ptr, descriptor, 0); }
	CURRENT_PROXY<L> back() { return CURRENT_PROXY<L>(ptr, descriptor, _capacity-1); }

	void swap(CURRENT_STATICARRAY &other) {
		std::swap(ptr, other.ptr);
		std::swap(_capacity, other._capacity);
		std::swap(descriptor, other.descriptor);
	}
};

using CURRENT_HYBRIDARRAY = CURRENT_STATICARRAY<LAYOUT>;

#undef ARRAY_NAME
#undef INITFIELDS

#undef CURRENT_PROXY 
#undef CURRENT_STATICARRAY 
#undef CURRENT_HYBRIDARRAY 
#undef CURRENT_OFFSETDESCRIPTOR 
#undef CURRENT_FIELDINDEX 
#undef CURRENT_AOSCELL

#else 
	#ifndef HYBRYDARRAY_HEADERS
		# error "HybridArray_headers.h must be included before the core file." 
	#endif

	#ifndef INITFIELDS
		# error "Define INITFIELDS macro before including HybridArray.h or HybrydArray_core." 
	#endif

	#ifndef LAYOUT
		# error "Define LAYOUT macro during compilation \nUSAGE: g++ myFile.cpp -D LAYOUT=[soa|aos]" 
	#endif
#endif