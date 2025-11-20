#ifdef HYBRIDARRAY_HEADERS

#ifndef INITFIELDS
	#error "ERROR: INITFIELDS macro not included.\n USAGE:\n 1.define INITFIELDS, and inside it add your desired Field or FieldArray fields.\n 2. include this library"
#endif

#ifndef LAYOUT
	#error "WARNING: LAYOUT not specified. \nIf you're using HybridArray, define it by adding -D LAYOUT=[soa|aos] during compilation.\nIf you're using StaticArray, ignore this message."
#endif

template<Layout L>
class Proxy;
template<Layout L>
class StaticArray;

/*Field _indexes, (size included)*/
enum Field_Index {
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
struct OffsetDescriptor {
	size_t offsets[Fields_Number];
	size_t totalSize;

	OffsetDescriptor(size_t _capacity) {
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
class AoSCell {
private:
	#define Field(type, name) type name;
	#define FieldArray(type, name, size) type name[size];

	INITFIELDS

	#undef Field
	#undef FieldArray

	friend class Proxy<aos>;
};

template <Layout L>
class Proxy {
private:
	void* ptr; // the previously allocated raw data
	const OffsetDescriptor& descriptor; // the previously created descriptor reference
	size_t _index;

public:
	Proxy(void* ptr, const OffsetDescriptor &descriptor, size_t _index)
		: ptr(ptr), descriptor(descriptor), _index(_index) {}

	// Getters and setters
	#define Field(type, name) \
		const type& get##name() const { \
			if constexpr (L == aos) { \
				AoSCell* base = static_cast<AoSCell*>(ptr); \
				return base[_index].name; \
			} else { \
				void* base = ptr; \
				type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]); \
				return field_ptr[_index]; \
			} \
		} \
		type& get##name() { \
			if constexpr (L == aos) { \
				AoSCell* base = static_cast<AoSCell*>(ptr); \
				return base[_index].name; \
			} else { \
				void* base = ptr; \
				type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]); \
				return field_ptr[_index]; \
			} \
		} \
		void set##name(const type &name) { \
			if constexpr (L == aos) { \
				AoSCell* base = static_cast<AoSCell*>(ptr); \
				base[_index].name = name; \
			} else { \
				void* base = ptr; \
				type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]); \
				field_ptr[_index] = name; \
			} \
		}

	#define FieldArray(type, name, size) \
		type* get##name() { \
			if constexpr (L == aos) { \
				AoSCell* base = static_cast<AoSCell*>(ptr); \
				return base[_index].name; \
			} else { \
				void* base = ptr; \
				type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]); \
				return field_ptr + (_index * size); \
			} \
		} \
		const type* get##name() const { \
			if constexpr (L == aos) { \
				AoSCell* base = static_cast<AoSCell*>(ptr); \
				return base[_index].name; \
			} else { \
				void* base = ptr; \
				type* field_ptr = reinterpret_cast<type*>(static_cast<char*>(base) + descriptor.offsets[IDX_##name]); \
				return field_ptr + (_index * size); \
			} \
		}

	INITFIELDS
	#undef Field
	#undef FieldArray
    
    size_t index() const{return _index;}

	friend class StaticArray<L>;
};

template<Layout L>
class StaticArray {
private:
	void* ptr;
	OffsetDescriptor descriptor;
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

	class iterator : public base_iterator<iterator, StaticArray<L>, Proxy<L>> {
		using base = base_iterator<iterator, StaticArray<L>, Proxy<L>>;
	public:
		iterator(StaticArray<L>& arr, size_t idx) : base(&arr, idx) {}
		iterator& operator++() { ++this->_index; return *this; }
		iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
		iterator& operator--() { --this->_index; return *this; }
		iterator operator--(int) { auto tmp = *this; --(*this); return tmp; }
		friend class const_iterator;
	};

	class const_iterator : public base_iterator<const_iterator, const StaticArray<L>, const Proxy<L>> {
		using base = base_iterator<const_iterator, const StaticArray<L>, const Proxy<L>>;
	public:
		const_iterator(const StaticArray<L>& arr, size_t idx) : base(&arr, idx) {}
		const_iterator(const iterator& it) : base(it.staticArray, it._index) {}
		const_iterator& operator++() { ++this->_index; return *this; }
		const_iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
		const_iterator& operator--() { --this->_index; return *this; }
		const_iterator operator--(int) { auto tmp = *this; --(*this); return tmp; }
	};
    using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
	StaticArray(size_t _capacity) :
		_capacity(_capacity),
		descriptor(_capacity),
		ptr(nullptr)
	{
		if constexpr(L == aos)
			ptr = new AoSCell[_capacity];
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

	StaticArray(const StaticArray& other) = delete;
	StaticArray& operator=(const StaticArray& other) = delete;

	~StaticArray() {
		if constexpr(L == aos)
			delete[] static_cast<AoSCell*>(ptr);
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

	Proxy<L> operator[](size_t _index) { return Proxy<L>(ptr, descriptor, _index); }
	Proxy<L> at(size_t _index) {
		if(_index >= _capacity)
			throw std::out_of_range("_Index specified needs to be less than _capacity.");
		return Proxy<L>(ptr, descriptor, _index);
	}
	Proxy<L> front() { return Proxy<L>(ptr, descriptor, 0); }
	Proxy<L> back() { return Proxy<L>(ptr, descriptor, _capacity-1); }

	void swap(StaticArray &other) {
		std::swap(ptr, other.ptr);
		std::swap(_capacity, other._capacity);
		std::swap(descriptor, other.descriptor);
	}
};

using HybridArray = StaticArray<LAYOUT>;

#endif