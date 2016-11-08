#include "vplotterrenderer.h"

VPlotterRenderer::VPlotterRenderer(QWidget *parent):QGraphicsView(new QGraphicsScene(),parent)
{
    p_scene = scene();
    //setBackgroundBrush(Qt::black);

    imgItem = p_scene->addPixmap(QPixmap());
    imgItem->setPos(0,0);
    plotterRectItem = p_scene->addRect(0,0,100,100);
}

