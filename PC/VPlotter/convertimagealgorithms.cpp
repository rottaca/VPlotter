#include "convertimagealgorithms.h"

#include "graphicseffects.h"
#include <QtMath>
#include <QMatrix>
#include "gcodecommands.h"

QStringList ConvertImageAlgorithms::convertLines(QImage &img, int angle, int threshold, int sampling
                                                 , QMatrix3x3 l2wTrans){
    QStringList cmds;
    cmds.append(GCODE_PEN_UP);

    int w = img.width();
    int h = img.height();
    int scanLineWidth = img.bytesPerLine();
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

    if(dirY > 0){
        bool drawing = false;
        for(int x = 0; x <w; x+= sampling){
            uchar* imgPtr = img.bits()+x;
            QVector2D posCurr(x,0);
            QStringList lineCmds;
            while(1){
                uchar c = *imgPtr;

                if(c > threshold){
                    if(!drawing){
                        QVector2D worldPos = convertLocalToWorld(posCurr,l2wTrans);
                        lineCmds.append(GCODE_SPEED_MOVE);
                        lineCmds.append(GCODE_MOVE_TO(worldPos.x(),worldPos.y()));
                        lineCmds.append(GCODE_PEN_DOWN);
                        lineCmds.append(GCODE_SPEED_DRAW);
                        drawing = true;
                    }
                }else{
                    if(drawing){
                        QVector2D worldPos = convertLocalToWorld(posCurr-QVector2D(dirX,dirY),l2wTrans);
                        lineCmds.append(GCODE_MOVE_TO(worldPos.x(),worldPos.y()));
                        lineCmds.append(GCODE_PEN_UP);
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
                QVector2D worldPos = convertLocalToWorld(posCurr-QVector2D(dirX,dirY),l2wTrans);
                lineCmds.append(GCODE_MOVE_TO(worldPos.x(),worldPos.y()));
                drawing = false;
            }
            // No line drawn?
            if(lineCmds.size() == 0)
                continue;

            // Invert commands for every second line to avoid unnecessary movements
            if((x/sampling) % 2 == 1)
                std::reverse(lineCmds.begin(),lineCmds.end());
            //cmds.append(";next line");
            cmds.append(GCODE_PEN_UP);
            for(int i = 0; i < lineCmds.size(); i++)
                cmds.append(lineCmds.at(i));

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
                float dy =0;
                if(abs(dx)<abs(a))
                    dy = qSqrt(a*a - dx*dx);

                posCurr.setX(w-1);
                posCurr.setY(y+dy);
                imgPtr = img.scanLine(y) + w-1;
            }else{
                posCurr.setX(0);
                posCurr.setY(y);
                imgPtr = img.scanLine(y);
            }

            QStringList lineCmds;
            while(1){
                uchar c = *imgPtr;

                if(c > threshold){
                    if(!drawing){
                        QVector2D worldPos = convertLocalToWorld(posCurr,l2wTrans);
                        lineCmds.append(GCODE_PEN_UP);
                        lineCmds.append(GCODE_SPEED_MOVE);
                        lineCmds.append(GCODE_MOVE_TO(worldPos.x(),worldPos.y()));
                        lineCmds.append(GCODE_PEN_DOWN);
                        lineCmds.append(GCODE_SPEED_DRAW);
                        drawing = true;
                    }
                }else{
                    if(drawing){
                        QVector2D worldPos = convertLocalToWorld(posCurr,l2wTrans);
                        lineCmds.append(GCODE_MOVE_TO(worldPos.x(),worldPos.y()));
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
                QVector2D worldPos = convertLocalToWorld(posCurr-QVector2D(dirX,dirY),l2wTrans);
                lineCmds.append(GCODE_MOVE_TO(worldPos.x(),worldPos.y()));
                drawing = false;
            }

            // No line drawn?
            if(lineCmds.size() == 0)
                continue;

            // Invert commands for every second line to avoid unnecessary movements
            if((y/sampling) % 2 == 1)
                std::reverse(lineCmds.begin(),lineCmds.end());

            //cmds.append(";next line");
            cmds.append(GCODE_PEN_UP);
            for(int i = 0; i < lineCmds.size(); i++)
                cmds.append(lineCmds.at(i));
        }
    }

    // Postprocess computed lines
    QList<int> indexList;
    QVector2D startPos(-INFINITY,-INFINITY),endPos(-INFINITY,-INFINITY);
    bool drawing = true;
    for(int i = 0; i < cmds.length(); i++){
        QString cmd = cmds.at(i);
        if(cmd.contains("M3")){
            if(drawing)
                indexList.append(i);
            drawing = true;
        }else if(cmd.contains("M4")){
            if(!drawing)
                indexList.append(i);
            drawing = false;
        }else if(cmd.contains("G0")){
            if(drawing){
                // TODO
            }
        }
    }

    // Remove bad elements
    for(int i = 0; i < indexList.length();i++){
        cmds.removeAt(indexList.at(i)-i);
    }


    return cmds;
}
QStringList ConvertImageAlgorithms::convertMultiLines(QImage img, bool* drawLines, bool mapToIntensity,
                                                      int sampling, int threshold,
                                                      QMatrix3x3 l2wTrans){
    QStringList cmds;

    // blur to remove small points
    img = GraphicsEffects::applyBlur(img);
    img = GraphicsEffects::applyBlur(img);

    int parts = 0;
    for(int i = 0; i < 4;i++)
        parts += drawLines[i]?1:0;

    int currPart = 1;

    if(mapToIntensity)
        threshold = 128;

    for(int i = 0; i < 4;i++){
        if(!drawLines[i])
            continue;
        QImage imgCpy;
        if(mapToIntensity)
            imgCpy = GraphicsEffects::applyBinarize(img,currPart*255/(parts+1),255,0);
        else
            imgCpy = img.copy();

        cmds.append(convertLines(imgCpy,45*i,threshold,sampling,l2wTrans));

        currPart++;
    }

    return cmds;
}

QStringList ConvertImageAlgorithms::convertWave(QImage img, float ySampling, int xSampling,
                                               QMatrix3x3 l2wTrans)
{

    QStringList cmds;
    for(int y = 0; y < img.height(); y= qRound(y+ySampling)){
        QVector2D worldPos = convertLocalToWorld(QVector2D(0,y),l2wTrans);
        cmds.append(GCODE_SPEED_DIV(1));
        cmds.append(GCODE_MOVE_TO(worldPos.x(),worldPos.y()));
        cmds.append(GCODE_PEN_DOWN);
        cmds.append(GCODE_SPEED_DIV(5));
        uchar* imgPtr = img.scanLine(y);

        float x = 0;
        // TODO flip direction every second row to remove unnecessary movements.
        while(x < img.width()){
            uchar c = imgPtr[qRound(x)];

            float width = (float)c/255*xSampling+2;
            QVector2D worldPos = convertLocalToWorld(QVector2D(x,y),l2wTrans);
            cmds.append(GCODE_MOVE_TO(worldPos.x(),worldPos.y()));
            worldPos = convertLocalToWorld(QVector2D(x+width/2,y),l2wTrans);
            cmds.append(GCODE_MOVE_TO(worldPos.x(),worldPos.y()));
            worldPos = convertLocalToWorld(QVector2D(x+width/2,y+ySampling-1),l2wTrans);
            cmds.append(GCODE_MOVE_TO(worldPos.x(),worldPos.y()));
            worldPos = convertLocalToWorld(QVector2D(x+width,y+ySampling-1),l2wTrans);
            cmds.append(GCODE_MOVE_TO(worldPos.x(),worldPos.y()));

            x+=width;
        }
        cmds.append(GCODE_PEN_UP);
    }
    return cmds;
}

QStringList ConvertImageAlgorithms::convertSquares(QImage img, int initialSize, int maxRecursion,QMatrix3x3 l2wTrans)
{
    QStringList cmds = createSquare(QVector2D(img.width()/2,img.height()/2),initialSize,l2wTrans);

    cmds.append(createSquaresRecursive(img,
                                  QVector2D(img.width()/2,img.height()/2),
                                  initialSize,
                                  maxRecursion,0,255,
                                  l2wTrans));
    return cmds;
}

QStringList ConvertImageAlgorithms::createSquaresRecursive(QImage& img, QVector2D center,
                                                           float size, int maxLayer, int layer, uchar intensityBorder,
                                                           QMatrix3x3 l2wTrans)
{
    QStringList cmds;
    cmds = createCross(center,size,l2wTrans);


    float s_2 = size/2;
    QRectF roi(center.x()- s_2,center.y()- s_2 ,size,size);

    float minIntensity = getMinimalIntensity(img,roi);
    if(layer > maxLayer || minIntensity > intensityBorder*(maxLayer-layer)/maxLayer)
        return cmds;

    float s_4 = s_2/2;
    layer++;
    cmds.append(createSquaresRecursive(img,center+QVector2D(-s_4,-s_4),s_2,maxLayer,layer,intensityBorder,l2wTrans));
    cmds.append(createSquaresRecursive(img,center+QVector2D(s_4,-s_4),s_2,maxLayer,layer,intensityBorder,l2wTrans));
    cmds.append(createSquaresRecursive(img,center+QVector2D(-s_4,s_4),s_2,maxLayer,layer,intensityBorder,l2wTrans));
    cmds.append(createSquaresRecursive(img,center+QVector2D(s_4,s_4),s_2,maxLayer,layer,intensityBorder,l2wTrans));

    return cmds;
}

QStringList ConvertImageAlgorithms::createSquare(QVector2D center, float size, QMatrix3x3 l2wTrans)
{
    QVector2D lu(center - QVector2D(size/2,size/2));
    QStringList cmds;
    QVector2D tmp = convertLocalToWorld(lu,l2wTrans);
    cmds.append(GCODE_SPEED_MOVE);
    cmds.append(GCODE_MOVE_TO(tmp.x(),tmp.y()));

    cmds.append(GCODE_PEN_DOWN);
    cmds.append(GCODE_SPEED_DRAW);
    tmp = convertLocalToWorld(lu+QVector2D(0,1)*size,l2wTrans);
    cmds.append(GCODE_MOVE_TO(tmp.x(),tmp.y()));
    tmp = convertLocalToWorld(lu+QVector2D(1,1)*size,l2wTrans);
    cmds.append(GCODE_MOVE_TO(tmp.x(),tmp.y()));
    tmp = convertLocalToWorld(lu+QVector2D(1,0)*size,l2wTrans);
    cmds.append(GCODE_MOVE_TO(tmp.x(),tmp.y()));
    tmp = convertLocalToWorld(lu+QVector2D(0,0)*size,l2wTrans);
    cmds.append(GCODE_MOVE_TO(tmp.x(),tmp.y()));
    cmds.append(GCODE_PEN_UP);
    return cmds;

}
QStringList ConvertImageAlgorithms::createCross(QVector2D center, float size, QMatrix3x3 l2wTrans)
{
    QVector2D lu(center - QVector2D(size/2,size/2));
    QStringList cmds;
    QVector2D tmp = convertLocalToWorld(lu+QVector2D(0.5,0)*size,l2wTrans);
    cmds.append(GCODE_MOVE_TO(tmp.x(),tmp.y()));

    cmds.append(GCODE_PEN_DOWN);
    tmp = convertLocalToWorld(lu+QVector2D(0.5,1)*size,l2wTrans);
    cmds.append(GCODE_MOVE_TO(tmp.x(),tmp.y()));
    cmds.append(GCODE_PEN_UP);
    tmp = convertLocalToWorld(lu+QVector2D(0,0.5)*size,l2wTrans);
    cmds.append(GCODE_MOVE_TO(tmp.x(),tmp.y()));
    cmds.append(GCODE_PEN_DOWN);
    tmp = convertLocalToWorld(lu+QVector2D(1,0.5)*size,l2wTrans);
    cmds.append(GCODE_MOVE_TO(tmp.x(),tmp.y()));
    cmds.append(GCODE_PEN_UP);
    return cmds;
}

QVector2D ConvertImageAlgorithms::convertLocalToWorld(QVector2D localPos, QMatrix3x3 l2wTrans)
{
    QGenericMatrix<1,3,float> pos;
    pos(0,0) = localPos.x();
    pos(1,0) = localPos.y();
    pos(2,0) = 1;
    pos = l2wTrans*pos;
    return QVector2D(pos(0,0),pos(1,0));
}

QVector2D ConvertImageAlgorithms::convertWorldToLocal(QVector2D worldPos, QMatrix3x3 w2lTrans)
{
    QGenericMatrix<1,3,float> pos;
    pos(0,0) = worldPos.x();
    pos(1,0) = worldPos.y();
    pos(2,0) = 1;
    pos = w2lTrans*pos;
    return QVector2D(pos(0,0),pos(1,0));
}

QMatrix3x3 ConvertImageAlgorithms::computeLocalToWorldTransform(QVector2D imgPos, float scale)
{
    QMatrix3x3 mat;
    mat.fill(0);
    mat(0,0) = scale;
    mat(1,1) = scale;
    mat(0,2) = imgPos.x();
    mat(1,2) = imgPos.y();
    mat(2,2) = 1;

    return mat;
}
uchar ConvertImageAlgorithms::getMinimalIntensity(QImage& img,QRectF roi)
{
    uchar min = 255;

    for(int y = 0; y < roi.height();y++){
        uchar* ptr = img.scanLine(roi.y())+qRound(roi.x());
        for(int x = 0; x < roi.width();x++){
            if(min > ptr[x])
                min = ptr[x];
        }
    }

    return min;
}
