#include "vplotterrenderer.h"
#include <QMouseEvent>

VPlotterRenderer::VPlotterRenderer(QWidget *parent):QGraphicsView(new QGraphicsScene(),parent)
{
    zoom = new Graphics_view_zoom(this);
    zoom->set_modifiers(Qt::NoModifier);

    p_scene = scene();
    setRenderHints( QPainter::SmoothPixmapTransform);
    drawBoardImgItem = p_scene->addPixmap(QPixmap());

    QPixmap motorL,motorR;
    if(!motorL.load(":/res/motor.png") || !motorR.load(":/res/motor.png")){
        qDebug("Can't load motor images.");
    }
    motorImgItemL = p_scene->addPixmap(motorL);
    motorImgItemR = p_scene->addPixmap(motorR);

    rawImgItem = p_scene->addPixmap(QPixmap());
    preprocessedImgItem = p_scene->addPixmap(QPixmap());
    imgItemSimulation = p_scene->addPixmap(QPixmap());

    setMotorPadding(60);
    renderPenUD = true;
    renderNonDrawMove = true;
    showItems(RAW);
    connect(&simulationTimer,SIGNAL(timeout()),this,SLOT(onSimulationTimerOverflow()));
}

void VPlotterRenderer::setPlotterSize(float w, float h){
    plotterSize.setX(w);
    plotterSize.setY(h);
    p_scene->setSceneRect(-100,-100,w+100,h+100);
    resetScale();

    int gridResMM = 100;
    int penWidth = 2;
    int pixmapPad = penWidth;
    QPixmap pixmap(w+2*pixmapPad,h+2*pixmapPad);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QPen gridPen((QColor(0,0,255)),penWidth,Qt::DashLine);  // Drawing
    QPen drawAreaPen((QColor(255,0,0)),penWidth);  // Drawing
    painter.setPen(gridPen);
    int lx, ly;
    lx = qFloor(w/gridResMM);
    ly = qFloor(h/gridResMM);

    QLine lines[lx+ly];
    int idx = 0;
    for(int y = 0; y <= ly;y++){
        lines[idx++] = QLine(pixmapPad,y*gridResMM+pixmapPad,
                             w+pixmapPad,y*gridResMM+pixmapPad);
    }
    for(int x = 0; x <= lx;x++){
        lines[idx++] = QLine(x*gridResMM+pixmapPad,pixmapPad,
                             x*gridResMM+pixmapPad,h+pixmapPad);
    }
    painter.drawLines(lines,idx);
    painter.setPen(drawAreaPen);
    painter.drawRect(motorPadding+pixmapPad,pixmapPad,
                     w-2*motorPadding,h);
    painter.end();
    drawBoardImgItem->setPixmap(pixmap);
    drawBoardImgItem->setPos(-pixmapPad,-pixmapPad);
    motorImgItemL->setPos(-motorImgItemL->pixmap().width()/2,-motorImgItemL->pixmap().height()/2);
    motorImgItemR->setPos(w-motorImgItemR->pixmap().width()/2,-motorImgItemR->pixmap().height()/2);
}

void VPlotterRenderer::simulateCommands(QStringList cmds)
{
    QPixmap pixmap(p_scene->width(),p_scene->height());
    pixmap.fill(Qt::transparent);

    pos = QVector2D(0,0);
    drawing = false;
    absolutePositioning = true;
    imgItemSimulation->setPixmap(pixmap);
    this->cmds = cmds;
    cmdIdx = 0;
    simulationTimer.start(0);
}
void VPlotterRenderer::abortSimulation()
{
    cmds.clear();
}

void VPlotterRenderer::onSimulationTimerOverflow(){
    QPixmap pxMap = imgItemSimulation->pixmap();
    QPainter painter(&pxMap);
    QPen Black((QColor(0,0,0)),1);  // Drawing
    QPen Red((QColor(255,0,0)),1);  // Not Drawing
    painter.setPen(Red);

    int drawCircleSize = 10;

    if(cmdIdx >= cmds.size()){
        simulationTimer.stop();
        emit onSimulationFinished();
        return;
    }
    do{
        QString cmd = cmds.at(cmdIdx++);

        if(cmd.contains("G0")){
            QVector2D newP;
            if(absolutePositioning){
                newP = readG0(cmd,pos);
            }else{
                newP = pos + readG0(cmd,QVector2D(0,0));
            }
            if(drawing){
                painter.setPen(Black);
                painter.drawLine((int)pos.x(),(int)pos.y(),(int)newP.x(),(int)newP.y());
            }
            else if(renderNonDrawMove){
                painter.setPen(Red);
                painter.drawLine((int)pos.x(),(int)pos.y(),(int)newP.x(),(int)newP.y());
            }
            pos = newP;
        }else if(cmd.contains("M3")){
            if(!drawing && renderPenUD){
                painter.setBrush(Qt::red);
                painter.setPen(Red);
                painter.drawEllipse((int)pos.x()-drawCircleSize/2,(int)pos.y()-drawCircleSize/2,
                                    drawCircleSize,drawCircleSize);
                painter.setBrush(Qt::NoBrush);
            }
            drawing = true;
        }else if(cmd.contains("M4")){
            if(drawing && renderPenUD){
                painter.setBrush(Qt::black);
                painter.setPen(Black);
                painter.drawEllipse((int)pos.x()-drawCircleSize/2,(int)pos.y()-drawCircleSize/2,
                                    drawCircleSize,drawCircleSize);
                painter.setBrush(Qt::NoBrush);
            }
            drawing = false;
        }else if(cmd.contains("G90")){
            absolutePositioning = true;
        }else if(cmd.contains("G91")){
            absolutePositioning = false;
        }
        // Very ugly :/
        else
            continue;
        break;

    }while(cmdIdx < cmds.length());
    imgItemSimulation->setPixmap(pxMap);

    if(cmdIdx >= cmds.length()){
        simulationTimer.stop();
        emit onSimulationFinished();
    }
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
