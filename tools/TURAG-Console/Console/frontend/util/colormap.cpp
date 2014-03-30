#include "colormap.h"
#include <cmath>
#include <QDebug>

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


QColor ColorMapDiscrete2::getColor(int index) {
    if (numberOfColors_ <= 1 || numberOfColors_ <= index) {
        return QColor(Qt::black);
    }

    int base = (int)(cbrt((float)(numberOfColors_ + 1)));
    int base_red = 0, base_green = 0, base_blue = 0;
    if (numberOfColors_ + 1 <= base * base * base) {
        base_blue = base;
        base_green = base;
        base_red = base;
    } else if (numberOfColors_ + 1 <= base * base * (base + 1)) {
        base_blue = base + 1;
        base_green = base;
        base_red = base;
    } else if (numberOfColors_ + 1 <= base * (base +1) * (base + 1)) {
        base_blue = base + 1;
        base_green = base + 1;
        base_red = base;
    } else if (numberOfColors_ + 1 <= (base +1) * (base +1) * (base + 1)) {
        base_blue = base + 1;
        base_green = base + 1;
        base_red = base + 1;
    }

    if (base_red == 1) {
        base_red = 2;
    }

    // calculate color parts
    int red = index / (base_blue * base_green);
    int green = (index - red * base_blue * base_green) / base_blue;
    int blue = index - red * base_blue * base_green - green * base_blue;

    // stretch color parts to rgb color area
    red = (int)((float)red * 255.0f / (base_red - 1));
    green = (int)((float)green * 255.0f / (base_green - 1));
    blue = (int)((float)blue * 255.0f / (base_blue - 1));

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


QColor ColorMapLinear2::getColor(int index) {
    if (numberOfColors_ <= 0 || numberOfColors_ <= index) {
        return QColor(Qt::black);
    }

    int red = 0, green = 0, blue = 0;

    int number_per_color = roundf((float)numberOfColors_ / 3.0f);
    int red_border = number_per_color;
    int green_border = number_per_color * 2;

    if (index < red_border) {
        red = (int)((float)index / (float)(number_per_color - 1) * 255.0f);
    } else if (index < green_border) {
        green = (int)((float)(index - red_border) / (float)(number_per_color - 1) * 255.0f);
    } else {
        blue = (int)((float)(index - green_border) / (float)(numberOfColors_ - green_border - 1) * 255.0f);
    }

    return QColor(red, green, blue);
}


