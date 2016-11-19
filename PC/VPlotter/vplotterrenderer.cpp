#include "vplotterrenderer.h"

VPlotterRenderer::VPlotterRenderer(QWidget *parent):QGraphicsView(new QGraphicsScene(),parent)
{
    p_scene = scene();

    rawImgItem = p_scene->addPixmap(QPixmap());
    preprocessedImgItem = p_scene->addPixmap(QPixmap());
    plotterRectItem = p_scene->addRect(0,0,100,100);
    imgItemDrawBoard = p_scene->addPixmap(QPixmap());
    imgItemSimulation = p_scene->addPixmap(QPixmap());

    renderPenUD = true;
    renderNonDrawMove = true;

    connect(&simulationTimer,SIGNAL(timeout()),this,SLOT(onSimulationTimerOverflow()));
}

void VPlotterRenderer::setPlotterSize(float w, float h){
    p_scene->setSceneRect(0,0,w,h);
    plotterRectItem->setRect(0,0,w,h);
    fitInView(p_scene->sceneRect(),Qt::KeepAspectRatio);

    int gridResMM = 100;

    QPixmap pixmap(w,h);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QPen Blue((QColor(0,0,255)),2,Qt::DashLine);  // Drawing
    painter.setPen(Blue);
    int lx, ly;
    lx = qFloor(w/gridResMM);
    ly = qFloor(h/gridResMM);

    QLine lines[lx+ly];
    int idx = 0;
    for(int y = 0; y < ly;y++){
        lines[idx++] = QLine(0,y*gridResMM,w,y*gridResMM);
    }
    for(int x = 0; x < lx;x++){
        lines[idx++] = QLine(x*gridResMM,0,x*gridResMM,h);
    }
    painter.drawLines(lines,idx);
    painter.end();
    imgItemDrawBoard->setPixmap(pixmap);
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
    QPen Black((QColor(0,0,0)),2);  // Drawing
    QPen Red((QColor(255,0,0)),2);  // Not Drawing
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
