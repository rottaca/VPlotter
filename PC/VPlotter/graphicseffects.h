#ifndef GRAPHICSEFFECTS_H
#define GRAPHICSEFFECTS_H

#include <QImage>

class GraphicsEffects
{
public:
    GraphicsEffects();


    static QImage applyBlur(QImage &input);
    static QImage applySobel(QImage &input);
};

#endif // GRAPHICSEFFECTS_H
