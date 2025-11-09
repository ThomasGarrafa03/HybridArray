#ifndef HYBRID_ARRAY
    #define HYBRID_ARRAY

    #include "HybridStructs_SingleProxy.h"

    #ifndef LAYOUT
        #error "\033[31mParameter not specified: specify whether to compile (g++) in SoA (by adding -D LAYOUT = soa) or AoS (by adding -D LAYOUT = aos).\033[0m")
    #endif

    using HybridArray = _HybridArray<LAYOUT>;

#endif