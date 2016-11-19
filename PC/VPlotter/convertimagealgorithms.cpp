#include "convertimagealgorithms.h"

#include "graphicseffects.h"
#include <QtMath>

QStringList ConvertImageAlgorithms::convertLines(QImage img, int angle, int threshold, int sampling
                                                 , QVector2D imgPos, float imgScale){
    QStringList cmds;
    cmds.append("M4");

    int w = img.width();
    int h = img.height();
    int scanLineWidth = w + (4 - w % 4);
    int sz = w*h;
    int dirX,dirY;

    switch(angle){
    case 0:
        dirX = 0;
        dirY = 1;
        break;
    case 45:
        dirX = 1;
        dirY = 1;
        break;
    case 90:
        dirX = 1;
        dirY = 0;
        break;
    case 135:
        dirX = -1;
        dirY = 1;
        break;
    }

    if(dirY != 0){
        bool drawing = false;
        for(int x = 0; x <w; x+= sampling){
            uchar* imgPtr = img.scanLine(0)+x;
            QVector2D posCurr(x,0);
            while(1){
                uchar c = *imgPtr;

                if(c > threshold){
                    if(!drawing){
                        QVector2D worldPos = convertLocalToWorld(posCurr,imgPos,imgScale);
                        cmds.append(QString("G0 X%1 Y%2").arg(worldPos.x()).arg(worldPos.y()));
                        cmds.append("M3");
                        drawing = true;
                    }
                }else{
                    if(drawing){
                        QVector2D worldPos = convertLocalToWorld(posCurr-QVector2D(dirX,dirY),imgPos,imgScale);
                        cmds.append(QString("G0 X%1 Y%2").arg(worldPos.x()).arg(worldPos.y()));
                        cmds.append("M4");
                        drawing = false;
                    }
                }

                imgPtr += scanLineWidth + dirX;
                posCurr.setX(posCurr.x()+dirX);
                posCurr.setY(posCurr.y()+dirY);

                if(posCurr.x()<0 ||posCurr.x()>=w || posCurr.y() >= h)
                    break;
            }

            if(drawing){
                QVector2D worldPos = convertLocalToWorld(posCurr-QVector2D(dirX,dirY),imgPos,imgScale);
                cmds.append(QString("G0 X%1 Y%2").arg(worldPos.x()).arg(worldPos.y()));
                cmds.append("M4");
                drawing = false;
            }
        }
    }

    if(dirX != 0){
        bool drawing = false;
        for(int y = 0; y < h; y+= sampling){
            uchar* imgPtr;

            QVector2D posCurr;
            if(dirX < 0){
                int dx = w%sampling;
                float a = qSin(qDegreesToRadians(45.0f))*sampling;
                posCurr.setX(w-1);
                posCurr.setY(y+qSqrt(a*a - dx*dx));
                imgPtr = img.scanLine(y) + w-1;
            }else{
                posCurr.setX(0);
                posCurr.setY(y);
                imgPtr = img.scanLine(y);
            }

            while(1){
                uchar c = *imgPtr;

                if(c > threshold){
                    if(!drawing){
                        QVector2D worldPos = convertLocalToWorld(posCurr,imgPos,imgScale);
                        cmds.append(QString("G0 X%1 Y%2").arg(worldPos.x()).arg(worldPos.y()));
                        cmds.append("M3");
                        drawing = true;
                    }
                }else{
                    if(drawing){
                        QVector2D worldPos = convertLocalToWorld(posCurr,imgPos,imgScale);
                        cmds.append(QString("G0 X%1 Y%2").arg(worldPos.x()).arg(worldPos.y()));
                        cmds.append("M4");
                        drawing = false;
                    }
                }

                imgPtr += dirY*scanLineWidth + dirX;
                posCurr.setX(posCurr.x()+dirX);
                posCurr.setY(posCurr.y()+dirY);

                if(posCurr.x()<0 ||posCurr.x()>=w || posCurr.y()>=h)
                    break;
            }

            if(drawing){
                QVector2D worldPos = convertLocalToWorld(posCurr-QVector2D(dirX,dirY),imgPos,imgScale);
                cmds.append(QString("G0 X%1 Y%2").arg(worldPos.x()).arg(worldPos.y()));
                cmds.append("M4");
                drawing = false;
            }
        }
    }


    return cmds;
}
QStringList ConvertImageAlgorithms::convertMultiLines(QImage img, bool* drawLines, bool mapToIntensity, int sampling,
                                                      QVector2D imgPos, float imgScale){
    QStringList cmds;

    int parts = 0;
    for(int i = 0; i < 3;i++)
        parts += drawLines[i]?1:0;

    int currPart = 1;

    for(int i = 0; i < 3;i++){
        if(!drawLines[i])
            continue;
        QImage imgCpy;
        if(mapToIntensity)
            imgCpy = GraphicsEffects::applyBinarize(img,currPart*255/(parts+1));
        else
            imgCpy = img.copy();

        cmds.append(convertLines(imgCpy,45*i,128,sampling,imgPos,imgScale));

        currPart++;
    }

    return cmds;
}

QVector2D ConvertImageAlgorithms::convertLocalToWorld(QVector2D localPos, QVector2D imgPos, float scale)
{
    return localPos*scale+imgPos;
}

QVector2D ConvertImageAlgorithms::convertWorldToLocal(QVector2D worldPos, QVector2D imgPos, float scale)
{
    return (worldPos-imgPos)/scale;
}
