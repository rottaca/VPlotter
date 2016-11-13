#include "graphicseffects.h"
#include <algorithm>
#include <cmath>


GraphicsEffects::GraphicsEffects()
{

}

QImage GraphicsEffects::applyBlur(QImage &input)
{
    if(input.format() != QImage::Format_Grayscale8)
        return QImage();

    QImage output(input.size(),QImage::Format_Grayscale8);
    double kernel[3][3] = {{1,2,1},{2,4,2},{1,2,1}};
    double kernelSum = 16;

    uchar *lines[3];
    lines[0]= input.scanLine(0);
    lines[1] = input.scanLine(1);
    lines[2] = input.scanLine(2);

    for (int y = 1; y < input.height()-1; y++) {
        lines[0] = lines[1];
        lines[1] = lines[2];
        lines[2] = input.scanLine(y+1);
        uchar* outputLine = output.scanLine(y);
        for (int x = 1; x < input.width()-1; x++) {
            double v = 0;

            for(int y_ = 0; y_ < 3; y_++)
                for(int x_=0; x_ < 3; x_++)
                    v += kernel[y_][x_]*lines[y_][x+x_-1];

            outputLine[x] = v/kernelSum;
        }
    }
    return output;
}

QImage GraphicsEffects::applySobel(QImage &input)
{
    if(input.format() != QImage::Format_Grayscale8)
        return QImage();

    double* intensity = new double[input.height()*input.width()];

    memset(intensity,0,sizeof(double)*input.height()*input.width());

    QImage output(input.size(),QImage::Format_Grayscale8);

    double kernelHoriz[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
    double kernelVert[3][3] = {{1,0,-1},{2,0,-2},{1,0,-1}};

    uchar *lines[3];
    lines[0]= input.scanLine(0);
    lines[1] = input.scanLine(1);
    lines[2] = input.scanLine(2);

    for (int y = 1; y < input.height()-1; y++) {
        lines[0] = lines[1];
        lines[1] = lines[2];
        lines[2] = input.scanLine(y+1);
        for (int x = 1; x < input.width()-1; x++) {
            double vH = 0;
            double vV = 0;

            for(int y_ = 0; y_ < 3; y_++){
                for(int x_=0; x_ < 3; x_++){
                    vH += kernelHoriz[y_][x_]*lines[y_][x+x_-1];
                    vV += kernelVert[y_][x_]*lines[y_][x+x_-1];
                }
            }

            intensity[y*input.width() + x] = sqrt(vH*vH+vV*vV);
        }
    }

    double maxIntens = intensity[0];
    for (int y = 1; y < input.height()-1; y++) {
        for (int x = 1; x < input.width()-1; x++) {
        if(maxIntens < intensity[y*input.width() + x])
            maxIntens = intensity[y*input.width() + x];
        }
    }

    for (int y = 1; y < input.height()-1; y++) {
        uchar* outputLine = output.scanLine(y);
        for (int x = 1; x < input.width()-1; x++) {
            outputLine[x] = intensity[y*input.width() + x]/maxIntens*255;
        }
    }
    delete intensity;

    return output;
}
