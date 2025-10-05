
#ifndef SCIDDICA_CELL
#define SCIDDICA_CELL

#define NUMBER_OF_OUTFLOWS 4 //config parameters: move into another file maybe
#define NEIGHBORHOOD_SIZE 5
#include <iostream>
using namespace std;

enum Dir { N, S, E, W };

//A SINGLE CELL
class SciddicaCell{
    private:
        static const int numberOfOutflows = NUMBER_OF_OUTFLOWS;
        double z; // quota del terreno
        double h; // altezza fluido
        double f[NUMBER_OF_OUTFLOWS]; // flussi uscenti verso i vicini (4 direzioni consentite)
        double minH = 0.0, maxH = 5.0;

    public:
        SciddicaCell() {
            this->z = 0.0;
            this->h = 0.0;
            for (int i = 0; i < numberOfOutflows; i++)
                f[i] = 0.0;
        }

        SciddicaCell(double z , double h, double f_in[numberOfOutflows]) {
            this->z = z;
            this->h = h;
            for (int i = 0; i < numberOfOutflows; i++)
                f[i] = (f_in ? f_in[i] : 0.0);
        }

        double getZ() const {return z;}
        void setZ(double val) {z = val;}

        double getH() const {return h;}
        void setH(double val) {h = val;}

        double getF(Dir d) const  {return f[d];}
        void setF(Dir d, double val) { f[d] = val;} 
};


#endif