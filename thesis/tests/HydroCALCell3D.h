//COPIED FROM "HydroCALCell3D.h"


#ifndef HydroCALCell_H
#define HydroCALCell_H


#include "../hybridArray/HybridArrayMarkers.h"

struct sub_cell
{
    double dqdh       ;
    double psi        ;
    double k          ;
    double h          ;
    double teta       ;
    double moist_cont ;
    double convergence;
};

struct sup_cell
{
    double psi        ;
    double h          ;
};

struct reduction_info
{
    //transiction function Superficial
    double tetaReinfiltrata        ; //ok
    double tetaEccessoInfiltrazione; //ok
    double tetaEccessoSaturazione  ; //ok
    double tetaInputPioggia        ; //ok
    double tetaSorgente            ; //ok
    double tetaOutlet              ; //ok

    //transiction function Subsurface
    double percolaz_falda          ; //ok
    double tetaExfiltrazione       ; //ok

    //endstep
    double tetaSuperficial         ;
    double tetaSubsurface          ;

    double celleTotSup          ;

};


class HydroCALCell3D
{
    public:
        FIELD double teta;
        FIELD double moist_cont;
        FIELD double psi;
        FIELD double intervol;
        FIELD double qout;
        FIELD double k;
        FIELD double ks;
        FIELD double h;
        FIELD double F[6];
        FIELD double FSup[9];
        FIELD double dqdh;
        FIELD double convergence;
        FIELD double contator;
        FIELD double timeact;
        FIELD double soilClass;
        FIELD double soilClassSuperficial;
        FIELD double source;
        FIELD double quota;
        FIELD sub_cell vicino_sotto;
        FIELD sup_cell vicino_sopra;
        FIELD reduction_info for_reduction;
        //rgb outputColor{0, 0, 0};
        double minH,maxH;	

        //REDUCTION

    public:
         HydroCALCell3D()
        {
        }

         HydroCALCell3D(double topSup,
                                double topSub,
                                double psi,
                                double quota,
                                double source,
                                double dqdh,        
                                double k,           
                                double h,           
                                double teta,        
                                double moist_cont,  
                                double convergence, 
                                double intervol,    
                                double qout        
)
        {
            this->soilClassSuperficial = topSup;
            this->soilClass = topSub;
            this->psi = psi;
            this->quota = quota;
            this->source = source;
            for (int i = 0; i < 6; i++)
                this->F[i] = 0;
            for (int i = 0; i < 9; i++)
                this->FSup[i] = 0;
            this->dqdh=dqdh;      
            this->k=k;           
            this->h=h;           
            this->teta=teta;        
            this->moist_cont=moist_cont;  
            this->convergence=convergence;
            this->intervol=intervol;    
            this->qout=qout;   
	          minH=0;
            maxH=700;
        }

         void setState()
        {
        }

         int getState()
        {
            return 0;
        }

        /*
 void composeElement(char *str)
        {
            //this->state = atoi(str);c
            // printf(" -------- composeElement \n");
            char* pComma = (char*)memchr(str,',', strlen(str));
	      		char* pSquare = str-1;
			      *pComma=0;
			      *pSquare=0;
			
			      this->h = atof(str+1);
			      this->teta = atof(pComma+1);
            
        }

        char *stringEncoding()
        {
            char* zstr = new char[512];
		       	sprintf(zstr,"[%0.6f,%0.6f]", h, teta);
			      return zstr;
        }

        rgb *outputValue(char* str)
        {
            // if (h > 0)
			// 	outputColor = rgb(1,0,0);
			// else
			// 	outputColor = rgb(1,1,1);
            // if (state == 0)
            //     outputColor = rgb(255, 0, 0);
            // if (state == 1)
            //     outputColor = rgb(0, 0, 0);
            //outputColor = rgb(0, 0, 0);
            //return &rgb(0, 0, 0);
            if(h > 0)
            {  
                outputColor = rgb((h-minH)*255/(maxH-minH),0,0);
            }
            else    
                outputColor = rgb(0, 0, 0);
            return &outputColor;
            
        }
        void startStep(int step){
            printf("startStep\n");
        }

     */
};
#endif
