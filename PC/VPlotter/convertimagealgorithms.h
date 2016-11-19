#ifndef CONVERTIMAGEEDGEDETECT_H
#define CONVERTIMAGEEDGEDETECT_H

#include <QStringList>
#include <QImage>
#include <QVector2D>

class ConvertImageAlgorithms
{
public:
    static QStringList convertLines(QImage img, int angle, int threshold, int sampling, QVector2D imgPos, float imgScale);
    static QStringList convertMultiLines(QImage img, bool* drawLines, bool mapToIntensity, int sampling, QVector2D imgPos, float imgScale);

private:
    static QVector2D convertLocalToWorld(QVector2D localPos, QVector2D imgPos, float scale);
    static QVector2D convertWorldToLocal(QVector2D worldPos, QVector2D imgPos, float scale);
};

#endif // CONVERTIMAGEEDGEDETECT_H
