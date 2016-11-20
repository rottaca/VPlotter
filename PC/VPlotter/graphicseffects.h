#ifndef GRAPHICSEFFECTS_H
#define GRAPHICSEFFECTS_H

#include <QImage>
#include <QVector>

class GraphicsEffects
{
public:


    static QImage applyBlur(QImage &input);
    static QImage applySobel(QImage &input);
    static QImage applyBinarize(QImage &input, uchar threshold, uchar below = 0, uchar aboveAndEq = 255);
    static QImage applyStretch(QImage &input, bool automatic=true, float quantile = 0.0f, int min=0, int max=255);

    static QVector<int> computeHist(QImage &input);
};

#endif // GRAPHICSEFFECTS_H
