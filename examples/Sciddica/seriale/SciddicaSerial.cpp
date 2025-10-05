#include "SciddicaCell.h"
#include <string.h>

#define P_EPSILON 0.001
#define P_R 0.5

#define NCOLS 30
#define NROWS 20
#define NSTEPS 4000

#define v(r,c) ((r)*NCOLS+(c))

AbstractSciddica* readM;
AbstractSciddica* writeM;

int main(int argc, char** argv){
    //check if parameter passing has been done correctly
    int status = checkArgs(argc, argv);
    if(status != 0) 
        return status;

    init(argv[1]);
    for(int step = 0; step<NSTEPS; step++){
        //print(step);
        transfunc(step);
        //swap();
    }
}

int checkArgs(int argc, char ** argv){
    if(argc == 0){
        cout << "Nessun parametro inserito: specifica se eseguire Sciddica attraverso SoA (soa) o AoS (aos)";
        return -1;
    };
    
    if(strcmp(argv[1], "soa") != 0 && strcmp(argv[1], "aos") != 0){
        cout << "Parametro errato: specifica se eseguire Sciddica attraverso SoA (soa) o AoS (aos)";
        return -2;
    }
    return 0;
}

void init(char *type){
    if(strcmp(type, "soa") == 0){
        readM = new SciddicaSOA(NROWS*NCOLS);
        writeM = new SciddicaSOA(NROWS*NCOLS);
    }
    else if(strcmp(type, "aos") == 0){
        readM = new SciddicaAOS();
        writeM = new SciddicaAOS();
    }

    for (int i = 0; i < NROWS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            int idx = v(i,j);

            readM->setZ(idx, (i+j)%100);         // quota artificiale
            readM->setH(idx, (i==200 && j==300) ? 10.0 : 0.0); // lava solo in un punto

            for (int d = 0; d < 4; d++)
                readM->setF(idx, (Dir)d, 0.0);
        }
    }
}

void transfunc(int step){
    for(int i = 0; i< NROWS; i++){
        for(int j = 0; j< NCOLS; j++){
            if(step & 1 == 1)
                sciddicaWidthUpdate(i,j);
            else
                sciddicaFlowsComputation(i,j);
        }
    }
}


void sciddicaFlowsComputation(int i, int j){
    bool eliminated_cells[NEIGHBORHOOD_SIZE];
    for(int n = 0; n< NEIGHBORHOOD_SIZE; n++)
        eliminated_cells[n] = false;

	bool again = false;
	int cells_count = 0, n = 0;
	double avarage = 0.0, m = 0.0, u[5] = {0.0, 0.0, 0.0, 0.0, 0.0}, z = 0.0, h = 0.0;		
		
	if(this->space->getCell(x,y).getH() <= P_EPSILON)
	{
		SciddicaTCell c = this->space->getCell(x, y);
        this->space->setCell(x, y, c);
		return;
	}
	
			m = this->space->getCell(x,y).getH() - P_EPSILON;

			u[0] = this->space->getCell(x,y).getZ() + P_EPSILON;
			u[1] = this->space->getCell(x-1,y).getZ() + this->space->getCell(x-1,y).getH();
			u[2] = this->space->getCell(x,y-1).getZ() + this->space->getCell(x,y-1).getH();
			u[3] = this->space->getCell(x,y+1).getZ() + this->space->getCell(x,y+1).getH();
			u[4] = this->space->getCell(x+1,y).getZ() + this->space->getCell(x+1,y).getH();

			if(dummyIteration!=0)
			    dummycomputation(dummyIteration);


			do{
				again = false;
				avarage = m;
				cells_count = 0;
			
				for(n=0; n<SIZE_OF_X; n++){
					if(!eliminated_cells[n]){
						avarage += u[n];
						cells_count++;
					}
				}
			
				if(cells_count != 0)
					avarage /= cells_count;
				
				for(n=0; n<SIZE_OF_X; n++){
					if((avarage <= u[n]) && (!eliminated_cells[n])){
						eliminated_cells[n] = true;
						again = true;
					}
				}
			} while(again);
			
			SciddicaTCell sciddicaTCell;
			SciddicaTCell old = this->space->getCell(x,y);
			sciddicaTCell.setH(old.getH());
			sciddicaTCell.setZ(old.getZ());
			
			for(n=1; n<SIZE_OF_X; n++){
				if(eliminated_cells[n]){
					sciddicaTCell.setF(0.0,n-1);
				}
				else{
					sciddicaTCell.setF((avarage-u[n])*P_R,n-1);
				}
			}
			
			this->space->setCell(x,y,sciddicaTCell);
}

void sciddicaWidthUpdate(int i, int j){

}