
enum Dir{
    N,
    S,
    E,
    W,
    DirNumber
};

#define INITFIELDS\
    Field(double, Z)\
    Field(double, H)\
    FieldArray(double, F, DirNumber)

#include "../../include/hybridArray/HybridArray.h"
#include "../../utils/ComputeTime.h"
#include <algorithm>
#include <cstdio>

#define TIMES 10

#define v(r,c) (r)*NCOLS+(c)
#define NROWS 1002
#define NCOLS 1002

#define NSTEPS 30
#define P_EPSILON 0.001
#define P_R 0.5

HybridArray readM(NROWS*NCOLS);
HybridArray writeM(NROWS*NCOLS);

void init(){
    for (int i = 0; i < NROWS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            int currIndex = v(i,j);
            readM[currIndex].setZ((i+j)%20);   // quota artificiale
            readM[currIndex].setH(i==NROWS/2 && j==NCOLS/2 ? 10.0 : 0.0);

            for(int d = 0; d<DirNumber; ++d)
                readM[currIndex].getF()[d] = 0.0;
        }
    }
}

void sciddicaFlowsComputation(int i, int j){
    int currIndex = v(i,j);
    int neighborhoodSize = DirNumber+1;
    bool eliminated_cells[neighborhoodSize];

    for(int n = 0; n< neighborhoodSize; n++)
        eliminated_cells[n] = false;

	bool again = false;
	int cells_count = 0, n = 0;
	double avarage = 0.0, m = 0.0, u[neighborhoodSize];

    for(int n = 0; n< neighborhoodSize; n++)
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
			
		for(n=0; n<neighborhoodSize; n++){
			if(!eliminated_cells[n]){
				avarage += u[n];
					cells_count++;
			}
		}
			
		if(cells_count != 0)
			avarage /= cells_count;
				
		for(n=0; n<neighborhoodSize; n++){
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
			
	for(n=1; n<neighborhoodSize; n++){
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
    int currIndex = v(i,j);
    double h_next = 0.0;

	h_next = readM[currIndex].getH();
	
    for(int di = -1; di<2; di++){
        for(int dj =-1; dj<2; dj++){
            //von neumann
            if(abs(di)+abs(dj) <= 1 && di != dj) {
                int neighborIndex = v(i + di, j + dj);

                double f_in = readM[neighborIndex].getF()[computeOppositeDirection(di, dj)];
                double f_out = readM[currIndex].getF()[computeDirection(di, dj)];

                h_next += f_in-f_out;
            }   
        }
    }
	
    writeM[currIndex].setZ( readM[currIndex].getZ() );

    double* readF = readM[currIndex].getF();
    double* writeF = writeM[currIndex].getF();
    std::copy(readF, readF + DirNumber, writeF);

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

void print(){
    printf("H: \n");
    for(int i = 1; i<NROWS-1; i++){
        for(int j = 1; j<NCOLS-1; j++){
            printf("%04.2f ", readM[v(i,j)].getH());
        }
        printf("\n");
    }

    printf("\n");
    printf("Z:\n");

    for(int i = 1; i<NROWS-1; i++){
        for(int j = 1; j<NCOLS-1; j++){
            printf("%04.0f ", readM[v(i,j)].getZ());
        }
        printf("\n");
    }

}

int main(){
    init();
    auto sciddica = [&](){
        for(int step = 0; step<NSTEPS; step++){
        //print(step);
        transfunc(step);
        readM.swap(writeM);
        }
    };
    
    double time = computeTime(sciddica, TIMES);

    printf("Average time elapsed: %f\n", time);
}

