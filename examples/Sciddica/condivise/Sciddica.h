#ifndef SCIDDICA
#define SCIDDICA

#include "SciddicaCell.h"
#include <iostream>
using namespace std;

template <class S>
class AbstractSciddica{
    protected:
        int size;
        double minH, maxH;

    public:
        AbstractSciddica<S>(int size){
            this->size = size;
            minH = 0.0;
            maxH = 5.0; //un numero arbitrario qualsiasi
        }
    //protected:
        //usare static_cast?

        double getZ(int i) const {return ((S*)this)->S::getZ(i);} 
        void setZ(int i, double val) {return ((S*)this)->S::setZ(i, val);}

        double getH(int i) const {return ((S*)this)->S::getH(i);}
        void setH(int i, double val) {return ((S*)this)->S::setH(i, val);}
        
        void getF(int i, double val[NUMBER_OF_OUTFLOWS]){((S*)this)->S::getF(i, val);}
        void setF(int i, const double val[NUMBER_OF_OUTFLOWS]){((S*)this)->S::setF(i, val);}
 
        double getF(int i, Dir d) const  {return ((S*)this)->S::getF(i, d);}
        void setF(int i, Dir d, double val) { return ((S*)this)->S::setF(i, d, val);} 

        int getSize(){return size;}
        double getMinH(){return minH;}
        double getMaxH(){return maxH;}
};

class SciddicaAoS: public AbstractSciddica<SciddicaAoS>{
    private:
        SciddicaCell* data;
    public:
        SciddicaAoS(int size):AbstractSciddica<SciddicaAoS>(size){
            data = new SciddicaCell[size];
        }

        ~SciddicaAoS() {
            delete[] data;
        }
        
        double getZ(int i) const {return data[i].getZ();}
        void setZ(int i, double val) {data[i].setZ(val);}

        double getH(int i) const {return data[i].getH();}
        void setH(int i, double val) { if(val > maxH) maxH = val; data[i].setH(val);}
        
        void getF(int i, double val[NUMBER_OF_OUTFLOWS]){
            for(int dir = 0; dir< NUMBER_OF_OUTFLOWS; dir++){
                val[dir] = getF(i,(Dir)dir);
            }
        }

        void setF(int i, const double val[NUMBER_OF_OUTFLOWS]){
            for(int dir = 0; dir< NUMBER_OF_OUTFLOWS; dir++){
                setF(i, (Dir)dir, val[dir]);
            }        
        }

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
        SciddicaSoA(int size):AbstractSciddica<SciddicaSoA>(size){
            z = new double[size]{0.0};
            h = new double[size]{0.0};

            // alloca per direzione
            f = new double*[NUMBER_OF_OUTFLOWS];
            for (int d = 0; d < NUMBER_OF_OUTFLOWS; d++)
                f[d] = new double[size]{0.0};
        }

        ~SciddicaSoA() {
            delete[] z;
            delete[] h;
            for (int d = 0; d < NUMBER_OF_OUTFLOWS; d++)
                delete[] f[d];
            delete[] f;
        }

        double getZ(int i) const { return z[i]; }
        void setZ(int i, double val) { z[i] = val; }

        double getH(int i) const { return h[i]; }
        void setH(int i, double val) { if(val > maxH) maxH = val; h[i] = val; }

        void getF(int i, double val[NUMBER_OF_OUTFLOWS]){
            for(int dir = 0; dir< NUMBER_OF_OUTFLOWS; dir++){
                val[dir] = getF(i,(Dir)dir);
            }
        }
        void setF(int i, const double val[NUMBER_OF_OUTFLOWS]){
            for(int dir = 0; dir< NUMBER_OF_OUTFLOWS; dir++){
                setF(i, (Dir)dir, val[dir]);
            }        
        }

        double getF(int i, Dir d) const { return f[d][i]; }
        void setF(int i, Dir d, double val) { f[d][i] = val; }
};

#endif