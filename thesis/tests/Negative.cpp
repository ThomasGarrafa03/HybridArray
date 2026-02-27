#include "../external/bitmap/libbmp.cpp"
#include <string>
#include <iostream>

using namespace std;

#define INITFIELDS\
    Field(char,R)\
    Field(char,G)\
    Field(char,B)\
    FieldArray(char,OFF, 30)

#include "../include/hybridArray/HybridArray.h"
#include "../utils/ComputeTime.h"

#define i_j i*cols +j

#define FILENAME_TOBLUR "Default.bmp"
#define FILENAME_BLUR "Negative.bmp"

void negateCell(int i, int cols, HybridArray &readM, HybridArray &writeM){
    int r = 0;
    int g = 0;
    int b = 0;    

    writeM[i].setR(255 - readM[i].getR());
    writeM[i].setG(255 - readM[i].getG());
    writeM[i].setB(255 - readM[i].getB());
}

int main(int argc, char ** argv) {
    BmpImg img;
    img.read(FILENAME_TOBLUR);

    int cols = img.get_width();
    int rows = img.get_height();

    //arrays initialization
    HybridArray readM(rows*cols);
    HybridArray writeM(rows*cols);
    
    for(int i = 0; i< rows; ++i){
        for(int j = 0; j< cols; ++j){
            auto cell = readM[i_j];
            cell.setR(img.red_at(j,i));
            cell.setG(img.green_at(j,i));
            cell.setB(img.blue_at(j,i));
        }
    }

    //negate!
    auto negate = [&](){
        for(int i = 0; i< rows*cols; ++i)
            negateCell(i, cols, readM, writeM);
    };
    
    double negateTime = computeTime(negate, 10);

    BmpImg toDraw(cols, rows);
    for(int i = 0; i< rows; ++i)
        for(int j = 0; j< cols; ++j)
            toDraw.set_pixel(j,i,writeM[i_j].getR(), writeM[i_j].getG(), writeM[i_j].getB());
    
    toDraw.write(FILENAME_BLUR);

    cout<< "Average negation time elapsed: " << negateTime <<"ms"<<endl;
            
    return 0;
}
