#include "../condivise/Sciddica.h"

#include <string.h>
#include <time.h>

Layout layout;

template <typename Layout>
void run(AbstractSciddica<Layout> *readM, AbstractSciddica<Layout> *writeM);
template <typename Layout>
void init(AbstractSciddica<Layout>* readM, AbstractSciddica<Layout>* writeM);
template<typename Layout>
void swap(AbstractSciddica<Layout> **readM, AbstractSciddica<Layout> **writeM);
template<typename Layout>
void print(int step, AbstractSciddica<Layout> *mat);
template<typename Layout>
void transfunc(int step, AbstractSciddica<Layout>* readM, AbstractSciddica<Layout>* writeM);
template<typename Layout>
void sciddicaFlowsComputation(int i, int j, AbstractSciddica<Layout>* readM, AbstractSciddica<Layout>* writeM);
template<typename Layout>
void sciddicaWidthUpdate(int i, int j, AbstractSciddica<Layout>*readM, AbstractSciddica<Layout>* writeM);

Dir computeDirection(int di, int dj);
Dir computeOppositeDirection(int di, int dj);

int main(int argc, char** argv){
    
    string l = (argc > 1)? argv[1]: "";

    if(l == "--soa"){
        layout = SoA;
        auto* readM = new SciddicaSoA(NROWS*NCOLS);
        auto* writeM = new SciddicaSoA(NROWS*NCOLS);
        run(readM, writeM);

        delete readM;
        delete writeM;
    }
    else if(l == "--aos"){
        layout = AoS;
        auto* readM = new SciddicaAoS(NROWS*NCOLS);
        auto* writeM = new SciddicaAoS(NROWS*NCOLS);
        run(readM, writeM);

        delete readM;
        delete writeM;
    }
    else{ cout<<"Parametro errato: specifica se eseguire Sciddica attraverso SoA (soa) o AoS (aos)"; return 1;}
}

template <typename Layout>
void run(AbstractSciddica<Layout> *readM, AbstractSciddica<Layout> *writeM){
    clock_t start = clock();
    init(readM,writeM);
    for(int step = 0; step<NSTEPS; step++){
        print(step, readM);
        transfunc(step, readM, writeM);
        swap(readM, writeM); //non proprio corretto, per via dell'altenrarsi degli step.
    }
    clock_t end = clock();
    double time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time elapsed: %f \n\n", time);
}

template <typename Layout>
void init(AbstractSciddica<Layout>* readM, AbstractSciddica<Layout>* writeM){

    for (int i = 0; i < NROWS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            int idx = v(i,j);

            readM->setZ(idx, rand()%(20));         // quota artificiale
            readM->setH(idx, (i==NROWS/2 && j==NCOLS/2) ? 10.0 : 0.0); // lava solo in un punto

            for (int d = 0; d < 4; d++)
                readM->setF(idx, (Dir)d, 0.0);
        }
    }
}

template<typename Layout>
void transfunc(int step, AbstractSciddica<Layout>* readM, AbstractSciddica<Layout>* writeM){
    for(int i = 0; i< NROWS-1; i++){
        for(int j = 0; j< NCOLS-1; j++){
            if(step %2 == 1){
                sciddicaWidthUpdate(i,j, readM, writeM);
            }else{
                sciddicaFlowsComputation(i,j, readM, writeM);
            }
        }
    }
}

template<typename Layout>
void sciddicaFlowsComputation(int i, int j, AbstractSciddica<Layout>* readM, AbstractSciddica<Layout>* writeM){
    int currIndex = v(i,j);
    bool eliminated_cells[NEIGHBORHOOD_SIZE];
    for(int n = 0; n< NEIGHBORHOOD_SIZE; n++)
        eliminated_cells[n] = false;

	bool again = false;
	int cells_count = 0, n = 0;
	double avarage = 0.0, m = 0.0, u[NEIGHBORHOOD_SIZE];

    for(int n = 0; n< NEIGHBORHOOD_SIZE; n++)
        u[n] = 0.0;
		
	if(readM->getH(v(i,j)) <= P_EPSILON)
	{   
        double f[NUMBER_OF_OUTFLOWS];
        readM->getF(currIndex, f);
        double h = readM->getH(currIndex);
        double z = readM->getZ(currIndex);

        writeM->setF(currIndex, f);
        writeM->setH(currIndex, h);
        writeM->setZ(currIndex, z);
        return;
	}
	
	m = readM->getH(currIndex) - P_EPSILON;
	u[0] = readM->getZ(currIndex) + P_EPSILON;

    //N,S,E,W
	u[1] = readM->getZ(v(i-1,j)) + readM->getH(v(i-1,j));
    u[2] = readM->getZ(v(i+1,j)) + readM->getH(v(i+1,j));
    u[3] = readM->getZ(v(i,j+1)) + readM->getH(v(i,j+1));
	u[4] = readM->getZ(v(i,j-1)) + readM->getH(v(i,j-1));

	do{
		again = false;
		avarage = m;
		cells_count = 0;
			
		for(n=0; n<NEIGHBORHOOD_SIZE; n++){
			if(!eliminated_cells[n]){
				avarage += u[n];
					cells_count++;
			}
		}
			
		if(cells_count != 0)
			avarage /= cells_count;
				
		for(n=0; n<NEIGHBORHOOD_SIZE; n++){
			if((avarage <= u[n]) && (!eliminated_cells[n])){
				eliminated_cells[n] = true;
				again = true;
			}
		}
        
	} while(again);
	
    double f[NUMBER_OF_OUTFLOWS];
    readM->getF(currIndex, f);
    double h = readM->getH(currIndex);
    double z = readM->getZ(currIndex);

	writeM->setH(currIndex,h);
	writeM->setZ(currIndex,z);
			
	for(n=1; n<NEIGHBORHOOD_SIZE; n++){
		if(eliminated_cells[n]){
            writeM->setF(currIndex,(Dir)(n-1),0.0);
		}
		else{
            writeM->setF(currIndex,(Dir)(n-1),(avarage-u[n])*P_R);
		}
	}
}

template<typename Layout>
void sciddicaWidthUpdate(int i, int j, AbstractSciddica<Layout>*readM, AbstractSciddica<Layout>* writeM){
    double h_next = 0.0;

	h_next = readM->getH(v(i,j));
	
    for(int di = -1; di<2; di++){
        for(int dj =-1; dj<2; dj++){
            if(abs(di)+abs(dj) <= 1 && di != dj) //von neumann
                h_next += readM->getF(v(i+di, j+dj),computeOppositeDirection(di, dj))-readM->getF(v(i,j),computeDirection(di,dj));
        }
    }
	
    writeM->setH(v(i,j),h_next);
}

template<typename Layout>
void swap(AbstractSciddica<Layout> **readM, AbstractSciddica<Layout> **writeM){
    AbstractSciddica<Layout> *temp = *readM;
    (*readM) = (*writeM);
    (*writeM) = temp;
}

template<typename Layout>
void print(int step, AbstractSciddica<Layout> *mat){
    printf("/** STEP: %d **/ \n", step);
    printf("FLUIDO:\n");
    for(int i = 0; i< NROWS-1; i++){
        for(int j = 0; j<NCOLS-1; j++){
            //printf("%f", mat->getMaxH());
            int val = (int) (((mat->getH(v(i,j)))-(mat->getMinH()))/((mat->getMaxH())-(mat->getMinH()))*255); //rappresentazione colore 0-255, basato su min e max.
            printf("%03d ", val);
        }
        printf("\n");
    }

    printf("ALTEZZE:\n");
    for(int i = 0; i< NROWS-1; i++){
        for(int j = 0; j<NCOLS-1; j++){
            //printf("%f", mat->getMaxH());
            int val = (int) mat->getZ(v(i,j));
            printf("%02d ", val);
        }
        printf("\n");
    }
    printf("\n");
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