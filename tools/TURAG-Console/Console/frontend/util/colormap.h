#ifndef COLORMAP_H
#define COLORMAP_H

#include <QColor>


class ColorMap
{
protected:
    int numberOfColors_;

public:
    ColorMap(int numberOfColors = 1) :
        numberOfColors_(numberOfColors) {}

    virtual QColor getColor(int index) = 0;
};


class ColorMapLinear : public ColorMap {
public:
    ColorMapLinear(int numberOfColors = 1) :
        ColorMap(numberOfColors) {}

    virtual QColor getColor(int index);
};


class ColorMapDiscrete : public ColorMap {
public:
    ColorMapDiscrete(int numberOfColors = 1) :
        ColorMap(numberOfColors) {}

    virtual QColor getColor(int index);
};

#endif // COLORMAP_H
