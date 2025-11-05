#ifndef OOCAL_ARRAY
    #define OOCAL_ARRAY

    #include "OOpenCALStructs.h"

    #ifndef LAYOUT
        #error "Parametro non specificato: specifica se compilare (g++) in SoA (aggiungendo -D LAYOUT = soa) o AoS (aggiungendo -D LAYOUT = aos).")
    #endif

    using OOpenCALArray = _OOpenCALArray<LAYOUT>;

#endif