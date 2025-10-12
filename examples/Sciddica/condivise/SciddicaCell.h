
#ifndef SCIDDICA_CELL
#define SCIDDICA_CELL

#include "SciddicaSettings.h"
#include <iostream>
using namespace std;

enum Dir { N, S, E, W };

//A SINGLE CELL
class SciddicaCell{
    private:
        double z; // quota del terreno
        double h; // altezza fluido
        double f[NUMBER_OF_OUTFLOWS]; // flussi uscenti verso i vicini (4 direzioni consentite)

    public:
        SciddicaCell() {
            this->z = 0.0;
            this->h = 0.0;
            for (int i = 0; i < NUMBER_OF_OUTFLOWS; i++)
                f[i] = 0.0;
        }

        SciddicaCell(double z , double h, double f_in[NUMBER_OF_OUTFLOWS]) {
            this->z = z;
            this->h = h;
            for (int i = 0; i < NUMBER_OF_OUTFLOWS; i++)
                f[i] = (f_in ? f_in[i] : 0.0);
        }

        double getZ() const {return z;}
        void setZ(double val) {z = val;}

        double getH() const {return h;}
        void setH(double val) {h = val;}

        void getF(double **f){(*f) = this->f;}
        void setF(double* f){
            for(int i = 0; i< NUMBER_OF_OUTFLOWS; i++)
                this->f[i] = f[i];
        }

        double getF(Dir d) const  {return f[d];}
        void setF(Dir d, double val) { f[d] = val;} 
};


#endif