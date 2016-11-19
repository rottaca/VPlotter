#ifndef GRAPHICSEFFECTS_H
#define GRAPHICSEFFECTS_H

#include <QImage>

class GraphicsEffects
{
public:
    GraphicsEffects();


    static QImage applyBlur(QImage &input);
    static QImage applySobel(QImage &input);
    static QImage applyBinarize(QImage &input, uchar threshold, uchar below = 0, uchar aboveAndEq = 255);
};

#endif // GRAPHICSEFFECTS_H
