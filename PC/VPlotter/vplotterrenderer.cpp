#include "vplotterrenderer.h"

VPlotterRenderer::VPlotterRenderer(QWidget *parent):QGraphicsView(new QGraphicsScene(),parent)
{
    p_scene = scene();

    imgItem = p_scene->addPixmap(QPixmap());
    imgItem->setPos(0,0);
    plotterRectItem = p_scene->addRect(0,0,100,100);
    imgItemSimulation = p_scene->addPixmap(QPixmap());
}


void VPlotterRenderer::simulateCommands(QStringList cmds)
{
    QPixmap pixmap(p_scene->width(),p_scene->height());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QPen Black((QColor(0,0,0)),2);  // Drawing
    QPen Red((QColor(255,0,0)),2);  // Not Drawing
    painter.setPen(Red);

    bool drawing = false;
    int drawCircleSize = 10;
    QVector2D pos(0,0);
    for(int i = 1; i < cmds.size();i++){
        QString cmd = cmds.at(i);
        qDebug(cmd.toLocal8Bit());
        if(cmd.contains("G0")){
            QVector2D newP = readG0(cmd,pos);
            if(drawing){
                painter.setBrush(Qt::NoBrush);
                painter.setPen(Black);
                painter.drawLine((int)pos.x(),(int)pos.y(),(int)newP.x(),(int)newP.y());
            }
            else{
                painter.setPen(Red);
                painter.drawLine((int)pos.x(),(int)pos.y(),(int)newP.x(),(int)newP.y());
            }
            pos = newP;
        }else if(cmd.contains("M3")){
            if(!drawing){
                painter.setBrush(Qt::red);
                painter.drawEllipse((int)pos.x()-drawCircleSize/2,(int)pos.y()-drawCircleSize/2,
                                    drawCircleSize,drawCircleSize);
            }
            drawing = true;
        }else if(cmd.contains("M4")){
            if(drawing){
                painter.setBrush(Qt::black);
                painter.drawEllipse((int)pos.x()-drawCircleSize/2,(int)pos.y()-drawCircleSize/2,
                                    drawCircleSize,drawCircleSize);
            }
            drawing = false;
        }
    }

    imgItemSimulation->setPixmap(pixmap);
}

QVector2D VPlotterRenderer::readG0(QString g0, QVector2D currPos)
{
    QVector2D newPos = currPos;
    QStringList params = g0.split((" "));
    for(int i = 1; i  < params.size();i++){
        QString p = params.at(i);
        if(p.contains("X")){
           p = p.remove(0,1);
            newPos.setX(p.toFloat());
        }else if(p.contains("Y")){
            p = p.remove(0,1);
            newPos.setY(p.toFloat());
         }
    }
    return newPos;
}
