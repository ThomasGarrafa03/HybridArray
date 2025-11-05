
enum Dir{
    N,
    S,
    E,
    W,
    DirNumber
};

#include "../include/stateless/OOpenCALArray.h"
#include <algorithm>

#define v(r,c) (r)*NCOLS+(c)
#define NROWS 10
#define NCOLS 5

#define NSTEPS 100
#define P_EPSILON 0.001
#define P_R 0.5

OOpenCALArray readM(NROWS*NCOLS);
OOpenCALArray writeM(NROWS*NCOLS);

void init(){
    for (int i = 0; i < NROWS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            int idx = v(i,j);

            readM[idx].setZ((i+j)%20);   // quota artificiale
            readM[idx].setH(i==NROWS/2 && j==NCOLS/2 ? 10.0 : 0.0);
            
            for(int d = 0; d<DirNumber; ++d)
                readM[idx].getF()[d] = 0.0;
        }
    }
}

void sciddicaFlowsComputation(int i, int j){
    int currIndex = v(i,j);
    bool eliminated_cells[DirNumber];

    for(int n = 0; n< DirNumber; n++)
        eliminated_cells[n] = false;

	bool again = false;
	int cells_count = 0, n = 0;
	double avarage = 0.0, m = 0.0, u[DirNumber];

    for(int n = 0; n< DirNumber; n++)
        u[n] = 0.0;
		
	if(readM[currIndex].getH() <= P_EPSILON)
	{   
        double *readF = readM[currIndex].getF();
        double h = readM[currIndex].getH();
        double z = readM[currIndex].getZ();

        double * writeF = writeM[currIndex].getF();
        std::copy(readF, readF+DirNumber, writeF);
        writeM[currIndex].setH(h);
        writeM[currIndex].setZ(z);

        return;
	}
	
	m = readM[currIndex].getH() - P_EPSILON;
	u[0] = readM[currIndex].getZ() + P_EPSILON;

    //N,S,E,W
	u[1] = readM[v(i-1,j)].getZ() + readM[v(i-1,j)].getH();
    u[2] = readM[v(i+1,j)].getZ() + readM[v(i+1,j)].getH();
	u[3] = readM[v(i,j+1)].getZ() + readM[v(i,j+1)].getH();
    u[4] = readM[v(i,j-1)].getZ() + readM[v(i,j-1)].getH();

	do{
		again = false;
		avarage = m;
		cells_count = 0;
			
		for(n=0; n<DirNumber; n++){
			if(!eliminated_cells[n]){
				avarage += u[n];
					cells_count++;
			}
		}
			
		if(cells_count != 0)
			avarage /= cells_count;
				
		for(n=0; n<DirNumber; n++){
			if((avarage <= u[n]) && (!eliminated_cells[n])){
				eliminated_cells[n] = true;
				again = true;
			}
		}
        
	} while(again);
	

    double *readF = readM[currIndex].getF();
    double h = readM[currIndex].getH();
    double z = readM[currIndex].getZ();

	writeM[currIndex].setH(h);
	writeM[currIndex].setZ(z);
			
	for(n=1; n<DirNumber; n++){
		if(eliminated_cells[n]){
            writeM[currIndex].getF()[n-1] = 0.0;
		}
		else{
            writeM[currIndex].getF()[n-1] = (avarage-u[n])*P_R;
		}
	}
}

Dir computeOppositeDirection(int di, int dj){
    if(di == -1) return S;
    if(di == 1) return N;
    if(dj == -1) return E;
    if(dj == 1) return W;
    return S;
}

Dir computeDirection(int di, int dj){
    if(di == -1) return N;
    if(di == 1) return S;
    if(dj == -1) return W;
    if(dj == 1) return E;
    return S;
}

void sciddicaWidthUpdate(int i, int j){
    double h_next = 0.0;

	h_next = readM[v(i,j)].getH();
	
    for(int di = -1; di<2; di++){
        for(int dj =-1; dj<2; dj++){
            if(abs(di)+abs(dj) <= 1 && di != dj) //von neumann
                h_next += readM[v(i,j)].getF()[computeOppositeDirection(di, dj)] - readM[v(i,j)].getF()[computeDirection(di,dj)];
        }
    }
	
    writeM[v(i,j)].setH(h_next);
}

void transfunc(int step){
    for(int i = 1; i< NROWS-1; i++){
        for(int j = 1; j< NCOLS-1; j++){
            if(step %2 == 1){
                sciddicaWidthUpdate(i,j);
            }else{
                sciddicaFlowsComputation(i,j);
            }
        }
    }
}

int main(){
    init();
    for(int step = 0; step<NSTEPS; step++){
        //print(step);
        transfunc(step);
        readM.swap(writeM);
    }
}