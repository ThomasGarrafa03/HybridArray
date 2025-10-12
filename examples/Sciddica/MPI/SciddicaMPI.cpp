/*
IDEA
MASTER: adibito alla sola stampa. Ad ogni step attende tutti gli altri processi per stamparne il risultato
SLAVE: adibiti al calcolo. Terminata la propria parte, invieranno al master il nuovo risultato.
*/

#include "../condivise/Sciddica.h"

#include <mpi/mpi.h>
#include <string.h>

#define SIZE (NROWS/NPROCS +2)*NCOLS

Dir computeDirection(int di, int dj);
Dir computeOppositeDirection(int di, int dj);

int rankMine, rankUp, rankDown, masterRank;
int nProc, nSlaveProcs;

Layout layout;
MPI_Datatype layout_datatype;

template <typename Layout>
void init(AbstractSciddica<Layout>* readM, AbstractSciddica<Layout>* writeM){

    for (int i = 0; i < NROWS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            int idx = v(i,j);

            readM->setZ(idx, rand()%(31));         // quota artificiale
            readM->setH(idx, (i==NROWS/2 && j==NCOLS/2) ? 10.0 : 0.0); // lava solo in un punto

            for (int d = 0; d < 4; d++)
                readM->setF(idx, (Dir)d, 0.0);
        }
    }
}

//creo il datatype corretto in base al layout dato (soa-aos)
MPI_Datatype createDataType(){
    MPI_Datatype mpi_layout_type;
    switch (layout){
        case SoA:
            const int nItems = 3;
            int blockLengths[nItems] = {NCOLS, NCOLS, NCOLS};
            MPI_Datatype types[nItems] = {MPI_DOUBLE,MPI_DOUBLE, MPI_DOUBLE};
            MPI_Aint offsets[3] = {0,8*NCOLS,16*NCOLS};

            MPI_Type_create_struct(nItems, blockLengths, offsets, types, &mpi_layout_type);
            MPI_Type_commit(&mpi_layout_type);
            break;

        case AoS:
            const int nItems = 3;
            int blockLengths[nItems] = {1,1,NUMBER_OF_OUTFLOWS};
            MPI_Datatype types[nItems] = {MPI_DOUBLE,MPI_DOUBLE, MPI_DOUBLE};
            MPI_Aint offsets[3] = {0,8,16};

            MPI_Type_create_struct(nItems, blockLengths, offsets, types, &mpi_layout_type);
            MPI_Type_commit(&mpi_layout_type);
            break;
    default:
        break;
    }    
}

template<typename Layout>
void print(int step, AbstractSciddica<Layout> *mat){
    printf("/** STEP: %d **/ \n", step);
    printf("FLUIDO:\n");
    for(int i = 0; i< NROWS-1; i++){
        for(int j = 0; j<NCOLS-1; j++){
            //printf("%f", mat->getMaxH());
            int val = (int) (((mat->getH(v(i,j)))-(mat->getMinH()))/((mat->getMaxH())-(mat->getMinH()))*255); //rappresentazione colore 0-255, basato su min e max.
            printf("%d ", val);
        }
        printf("\n");
    }

    printf("ALTEZZE:\n");
    for(int i = 0; i< NROWS-1; i++){
        for(int j = 0; j<NCOLS-1; j++){
            //printf("%f", mat->getMaxH());
            int val = (int) mat->getZ(v(i,j));
            printf("%d ", val);
        }
        printf("\n");
    }
    printf("\n");
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
void transfunc(int step, AbstractSciddica<Layout>* readM, AbstractSciddica<Layout>* writeM){
    //NO halo cells: 1 -> nrows/nprocs +1
    for(int i = 1; i< NROWS/NPROCS+1; i++){
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
void swap(AbstractSciddica<Layout> **readM, AbstractSciddica<Layout> **writeM){
    AbstractSciddica<Layout> *temp = *readM;
    (*readM) = (*writeM);
    (*writeM) = temp;
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

void masterWork(){
    //MASTER: attende che riceva i dati dagli slave, per poi stamparli (non il massimo)
    AbstractSciddica<Layout> *recieved;
    MPI_Status status;

    for(int i = 0; i< nProc; i++){
        MPI_Recv(recieved, nProc, layout_datatype, MPI_ANY_SOURCE, 69, MPI_COMM_WORLD, &status);
        
    }

}

template<typename Layout>
void slaveWork(int step, AbstractSciddica<Layout> *readM, AbstractSciddica<Layout> *writeM){
    //SLAVE: esegue il lavoro effettivo
}

template <typename Layout>
void run(AbstractSciddica<Layout> *readM, AbstractSciddica<Layout> *writeM){    
    double starttime = MPI_Wtime();

    init(readM,writeM);
    for(int step = 0; step<NSTEPS; step++){
        if(rankMine == masterRank)
            masterWork(step);
        else
            slaveWork(step, readM, writeM);
    }
    double endtime = MPI_Wtime();

    if(rank == nProc) printf("Elapsed time: %f\n", 1000*(endtime-starttime);
    printf("Time elapsed: %f \n\n", time);
}

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nProc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rankMine);

    //utilizzo un processo per la stampa: il numero di processi totali è quindi diminuito di 1.
    nProc -=1;
    //l'ultimo processo sarà adibito a master
    masterRank = nProc;

    rankUp = (rankMine-1+nProc)%nProc;
    rankDown = (rankMine+1)%nProc;
    
    string l = (argc > 1)? argv[1]: "";

    if(l == "--soa"){
        layout = SoA;
        auto* readM = new SciddicaSoA(SIZE);
        auto* writeM = new SciddicaSoA(SIZE);
        layout_datatype = createDataType();
        run(readM, writeM);

        delete readM;
        delete writeM;
    }
    else if(l == "--aos"){
        layout = AoS;
        auto* readM = new SciddicaAoS(SIZE);
        auto* writeM = new SciddicaAoS(SIZE);
        layout_datatype = createDataType();
        run(readM, writeM);

        delete readM;
        delete writeM;
    }
    else{ cout<<"Parametro errato: specifica se eseguire Sciddica attraverso SoA (soa) o AoS (aos)"; return 1;}
}