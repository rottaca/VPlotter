#ifndef VPLOTTERRENDERER_H
#define VPLOTTERRENDERER_H

#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QVector2D>
#include <QtMath>
#include <QLine>
#include <QTimer>

#include "graphics_view_zoom.h"
class VPlotterRenderer : public QGraphicsView
{
    Q_OBJECT
public:
    VPlotterRenderer(QWidget *parent = 0);

    void setRawImage(QImage img){
        rawImgItem->setPixmap(QPixmap::fromImage(img));
    }
    void setPreprocessedImage(QImage img){
        preprocessedImgItem->setPixmap(QPixmap::fromImage(img));
    }

    void setImageBounds(float scale, float x, float y){
        rawImgItem->setPos(x,y);
        rawImgItem->setScale(scale);
        preprocessedImgItem->setPos(x,y);
        preprocessedImgItem->setScale(scale);
    }

    void setPlotterSize(float w, float h);

    QVector2D getPlotterSize(){
        return QVector2D(p_scene->sceneRect().width(),p_scene->sceneRect().height());
    }

    void resizeEvent(QResizeEvent * event){
        fitInView(p_scene->sceneRect(),Qt::KeepAspectRatio);
    }

    void setRenderOptions(bool renderNonDraw, bool renderUD){
        renderNonDrawMove = renderNonDraw;
        renderPenUD = renderUD;
        // Rerun rendering
        simulateCommands(cmds);
    }

    enum DisplayMode{RAW,PREPROC,SIMULATION};
    void showItems(enum DisplayMode mode){
        dispMode = mode;
        rawImgItem->setVisible(false);
        preprocessedImgItem->setVisible(false);
        imgItemSimulation->setVisible(false);
        if(mode == RAW){
            rawImgItem->setVisible(true);
        }else if(mode == PREPROC){
            preprocessedImgItem->setVisible(true);
        }else{
            imgItemSimulation->setVisible(true);
        }
    }

public slots:
    void onSimulationTimerOverflow();
    void simulateCommands(QStringList cmds);
    void abortSimulation();

signals:
    void onSimulationFinished();

private:
    QVector2D readG0(QString g0, QVector2D currPos);


private:
    Graphics_view_zoom* zoom;
    QGraphicsPixmapItem* rawImgItem;
    QGraphicsPixmapItem* preprocessedImgItem;
    QGraphicsRectItem* plotterRectItem;
    QGraphicsPixmapItem* imgItemSimulation;
    QGraphicsPixmapItem* imgItemDrawBoard;

    QVector2D imgSize;
    QRect imgBounds;

    QGraphicsScene* p_scene;

    bool renderPenUD;
    bool renderNonDrawMove;
    QTimer simulationTimer;
    QStringList cmds;
    int cmdIdx;
    bool drawing;
    bool absolutePositioning;
    QVector2D pos; // Start postion 0,0
    enum DisplayMode dispMode;
};

#endif // VPLOTTERRENDERER_H
