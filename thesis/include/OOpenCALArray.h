#ifndef OOCAL_ARRAY
    #define OOCAL_ARRAY

    #include "OOpenCALStructs.h"

    #ifndef LAYOUT
        #error "Parametro non specificato: specifica se compilare (g++) in SoA (aggiungendo -D LAYOUT = soa) o AoS (aggiungendo -D LAYOUT = aos).")
    #endif

    // Defines LAYOUT for IntelliSense only in order to remove "undefined" error (comment this code and see!).
    // It's totally ignored by the real compiler (g++).
    #ifdef __INTELLISENSE__
        #define LAYOUT soa // or AoS
    #endif

    using OOpenCALArray = _OOpenCALArray<LAYOUT>;

#endif