/**
 *  Copyright 2020 Mike Reed
 */

#include "image_final.cpp"

const GDrawRec gDrawRecs[] = {
    { final_bilerp, 512, 512, "final_bilerp", 1 },
    { final_radial, 512, 512, "final_radial", 1 },
    { final_stroke, 512, 512, "final_stroke", 1 },
    { final_rrect,  512, 512, "final_rrect",  1 },

    { nullptr, 0, 0, nullptr },
};
