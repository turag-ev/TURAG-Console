#ifndef COLORMAP_H
#define COLORMAP_H

#include <QColor>


class ColorMap
{
public:
    ColorMap(int numberOfColors = 1) :
        numberOfColors_(numberOfColors) {}

    virtual ~ColorMap(void) {}

    virtual QColor getColor(int index) = 0;

protected:
    int numberOfColors_;

};


class ColorMapLinear : public ColorMap {
public:
    ColorMapLinear(int numberOfColors = 1) :
        ColorMap(numberOfColors) {}
    virtual ~ColorMapLinear(void) {}

    virtual QColor getColor(int index);
};


class ColorMapLinear2 : public ColorMap {
public:
    ColorMapLinear2(int numberOfColors = 1) :
        ColorMap(numberOfColors) {}
    virtual ~ColorMapLinear2(void) {}

    virtual QColor getColor(int index);
};


class ColorMapDiscrete : public ColorMap {
public:
    ColorMapDiscrete(int numberOfColors = 1) :
        ColorMap(numberOfColors) {}
    virtual ~ColorMapDiscrete(void) {}

    virtual QColor getColor(int index);
};

class ColorMapDiscrete2 : public ColorMap {
public:
    ColorMapDiscrete2(int numberOfColors = 1) :
        ColorMap(numberOfColors) {}
    virtual ~ColorMapDiscrete2(void) {}

    virtual QColor getColor(int index);
};

#endif // COLORMAP_H
