#include "colormap.h"
#include <cmath>


QColor ColorMapDiscrete::getColor(int index) {
    if (numberOfColors_ <= 1 || numberOfColors_ <= index) {
        return QColor(Qt::black);
    }

    // we calculate how many times we have to divide the three
    // rgb color parts to get a sufficient amount of colors.
    // we increase the amount by one, because we exclude white.
    int base = (int)::ceil(cbrt((float)(numberOfColors_ + 1)));

    // calculate color parts
    int red = index / (base * base);
    int green = (index - red * base * base) / base;
    int blue = index - red * base * base - green * base;

    // stretch color parts to rgb color area
    float stretch_factor = 255.0f / (base - 1);
    red = (int)((float)red * stretch_factor);
    green = (int)((float)green * stretch_factor);
    blue = (int)((float)blue * stretch_factor);

    return QColor(red, green, blue);
}



QColor ColorMapLinear::getColor(int index) {
    if (numberOfColors_ <= 0 || numberOfColors_ <= index) {
        return QColor(Qt::black);
    }

    float color_value = 256.0f * 256.0f * 256.0f / (float)numberOfColors_ * (float)index;

    // calculate color parts
    int red = (int)(color_value / (256.0f * 256.0f));
    int green = (int)((color_value - (float)red * 256.0f * 256.0f) / 256.0f);
    int blue = (int)(color_value - (float)red * 256.0f * 256.0f - (float)green * 256.0f);

    return QColor(red, green, blue);
}


