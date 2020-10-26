#ifndef DRAWING_H
#define DRAWING_H

#include "Image.h"

namespace cli
{
    void draw_borders();
    void update_elipsis(double);
    void update_colour(Colour, SHORT, double, int);
    void update_main_table(int, double, int, Colour, SHORT);
}

#endif  // DRAWING_H