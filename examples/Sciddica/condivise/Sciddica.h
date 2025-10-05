#ifndef SCIDDICA
#define SCIDDICA

#include "SciddicaCell.h"
#include <iostream>
using namespace std;

template <class S>
class AbstractSciddica{
    protected:
        static const int numberOfOutflows = NUMBER_OF_OUTFLOWS;

    public:
    //protected:
        //usare static_cast?

        double getZ(int i) const {return ((S*)this)->S::getZ(i)} 
        void setZ(int i, double val) {return ((S*)this)->S::setZ(i, val)}

        double getH(int i) const {return ((S*)this)->S::getH(i)}
        void setH(int i, double val) {return ((S*)this)->S::setH(i, val)}

        double getF(int i, Dir d) const  {return ((S*)this)->S::getF(i, d)}
        void setF(int i, Dir d, double val) { return ((S*)this)->S::setF(i, d, val)} 
};

class SciddicaAoS: public AbstractSciddica<SciddicaAoS>{
    private:
        SciddicaCell* data;
    public:
        SciddicaAoS(int size){
            data = new SciddicaCell[size];
        }

        ~SciddicaAoS() {
            delete[] data;
        }
        
        double getZ(int i) const {return data[i].getZ();}
        void setZ(int i, double val) {data[i].setZ(val);}

        double getH(int i) const {return data[i].getH();}
        void setH(int i, double val) {data[i].setH(val);}

        double getF(int i, Dir d) const  {return data[i].getF(d);}
        void setF(int i, Dir d, double val) { data[i].setF(d,val);} 
};

class SciddicaSoA: public AbstractSciddica<SciddicaSoA>{
    private:
        double *z;
        double *h;
        double **f;
        //double minH = 0.0, maxH = 5.0;

    public:
        SciddicaSoA(int size){
            z = new double[size];
            h = new double[size];

            // alloca per direzione
            f = new double*[numberOfOutflows];
            for (int d = 0; d < numberOfOutflows; d++)
                f[d] = new double[size]{0.0};
        }

        ~SciddicaSoA() {
            delete[] z;
            delete[] h;
            for (int d = 0; d < numberOfOutflows; d++)
                delete[] f[d];
            delete[] f;
        }

        double getZ(int i) const { return z[i]; }
        void setZ(int i, double val) { z[i] = val; }

        double getH(int i) const { return h[i]; }
        void setH(int i, double val) { h[i] = val; }

        double getF(int i, Dir d) const { return f[d][i]; }
        void setF(int i, Dir d, double val) { f[d][i] = val; }
};

#endif