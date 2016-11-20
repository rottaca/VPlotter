#include "graphicseffects.h"
#include <algorithm>
#include <cmath>



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
QImage GraphicsEffects::applyBinarize(QImage &input, uchar threshold, uchar below, uchar aboveAndEq)
{
    if(input.format() != QImage::Format_Grayscale8)
        return QImage();

    QImage output(input.size(),QImage::Format_Grayscale8);


    for (int y = 0; y < input.height(); y++) {
        uchar* inPtr = input.scanLine(y);
        uchar* outPtr = output.scanLine(y);
        for (int x = 0; x < input.width(); x++) {
            uchar v = inPtr[x];
            if(v<threshold){
                outPtr[x] = below;
            }else{
                outPtr[x] = aboveAndEq;
            }
        }
    }
    return output;
}

QImage GraphicsEffects::applyStretch(QImage &input,bool automatic, float quantile,int min, int max)
{
    if(input.format() != QImage::Format_Grayscale8)
        return QImage();

    QImage output(input.size(),QImage::Format_Grayscale8);

    if(automatic){
        QVector<int> hist = computeHist(input);
        min = 0;
        max = 255;
        int minSum = 0;
        int maxSum = 0;
        int w = input.width();
        int h = input.height();
        while(true){
            minSum += hist[min];
            if(minSum>= w*h*quantile || min >255)
                break;
            min++;
        }
        while(true){
            maxSum += hist[max];
            if(maxSum>= w*h*quantile || max <0)
                break;

            max--;
        }
    }
    qDebug(QString("Min: %1 \n Max: %2").arg(min).arg(max).toLocal8Bit());

    if(min >= max){
        qDebug("Stretch failed!");
        return output;
    }

    for (int y = 0; y < input.height(); y++) {
        uchar* inPtr = input.scanLine(y);
        uchar* outPtr = output.scanLine(y);
        for (int x = 0; x < input.width(); x++) {
            uchar v = inPtr[x];
            if(v<=min){
                outPtr[x] = 0;
            }else if(v>=max){
                outPtr[x] = 255;
            }else{
                outPtr[x] = 255*(v-min)/(max-min);
            }
        }
    }
    return output;
}
QVector<int> GraphicsEffects::computeHist(QImage &input)
{
    QVector<int> vec(256,0);
    for (int y = 0; y < input.height(); y++) {
        uchar* inPtr = input.scanLine(y);
        for (int x = 0; x < input.width(); x++) {
            uchar v = inPtr[x];
            vec[v]++;
        }
    }

    return vec;
}
