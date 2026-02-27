#include "../external/bitmap/libbmp.cpp"
#include <string>
#include <iostream>

using namespace std;

#define INITFIELDS\
    Field(char,R)\
    Field(char,G)\
    Field(char,B)

#include "../include/hybridArray/HybridArray.h"
#include "../utils/ComputeTime.h"

#define i_j i*cols +j

#define FILENAME_TOBLUR "Default.bmp"
#define FILENAME_BLUR "Blurred.bmp"

void blurCell(int i, int j, int cols, HybridArray &readM, HybridArray &writeM){
    int r = 0;
    int g = 0;
    int b = 0;

    for(int di = -1; di < 2; ++di){
        for(int dj = -1; dj < 2; ++dj){
            int off = (i+di)*cols + (j+dj);
            r += readM[off].getR();
            g += readM[off].getG();
            b += readM[off].getB();               
        }
    }        

    r /= 9;
    g /= 9;
    b /= 9;

    writeM[i_j].setR(r);
    writeM[i_j].setG(g);
    writeM[i_j].setB(b);
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
            readM[i_j].setR(img.red_at(j,i));
            readM[i_j].setG(img.green_at(j,i));
            readM[i_j].setB(img.blue_at(j,i));
        }
    }

    //blur!
    auto blur = [&](){
        for(int i = 0; i< rows; ++i)
        for(int j = 0; j< cols; ++j)
            blurCell(i,j, cols, readM, writeM);
    };
    
    double blurTime = computeTime(blur, 10);

    BmpImg toDraw(cols, rows);
    for(int i = 0; i< rows; ++i)
        for(int j = 0; j< cols; ++j)
            toDraw.set_pixel(j,i,writeM[i_j].getR(), writeM[i_j].getG(), writeM[i_j].getB());
    
    toDraw.write(FILENAME_BLUR);

    cout<< "Average blur time elapsed: " << blurTime <<"ms"<<endl;
            
    return 0;
}
