#ifndef HYBRIDARRAY_HEADERS
#define HYBRIDARRAY_HEADERS

#include <cstdlib> //for malloc, free, ...
#include <type_traits> //for is_trivially_destructible_v<V>
#include <new> //for placement new
#include <algorithm> //for swap
#include <stdexcept>

/*Layout used*/
enum Layout{
    soa,
    aos
};

#endif
