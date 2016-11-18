#ifndef VPLOTTERRENDERER_H
#define VPLOTTERRENDERER_H

#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QVector2D>

class VPlotterRenderer : public QGraphicsView
{
public:
    VPlotterRenderer(QWidget *parent = 0);

    void setImage(QImage img){
        imgItem->setPixmap(QPixmap::fromImage(img));
    }

    void setImageBounds(float scale, float x, float y){
        imgItem->setPos(x,y);
        imgItem->setScale(scale);
    }

    void setPlotterSize(float w, float h){
        p_scene->setSceneRect(0,0,w,h);
        plotterRectItem->setRect(0,0,w,h);
        fitInView(p_scene->sceneRect(),Qt::KeepAspectRatio);
    }

    QVector2D getPlotterSize(){
        return QVector2D(p_scene->sceneRect().width(),p_scene->sceneRect().height());
    }

    void resizeEvent(QResizeEvent * event){
        fitInView(p_scene->sceneRect(),Qt::KeepAspectRatio);
    }

    void simulateCommands(QStringList cmds);
private:
    QVector2D readG0(QString g0, QVector2D currPos);

private:
    QGraphicsPixmapItem* imgItem;
    QGraphicsRectItem* plotterRectItem;
    QVector2D imgSize;
    QRect imgBounds;
    QGraphicsPixmapItem* imgItemSimulation;

    QGraphicsScene* p_scene;

};

#endif // VPLOTTERRENDERER_H
