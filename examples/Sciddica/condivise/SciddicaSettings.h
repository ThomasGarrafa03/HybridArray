#define NUMBER_OF_OUTFLOWS 4 
#define NEIGHBORHOOD_SIZE 5
#define P_EPSILON 0.001
#define P_R 0.5

//righe e colonne effettive
#define NCOLS 7
#define NROWS 7

#define NSTEPS 30

#define v(r,c) (r)*NCOLS+(c)

//PARALLELO
#define NPROCS 4

enum Layout{
    SoA,
    AoS
};