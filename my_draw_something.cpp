/*
 *  Copyright 2022 <Claire Helms>
 */

#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GRandom.h"
#include "GPoint.h"
#include "GRect.h"
#include "GShader.h"
#include "GPath.h"
#include <string>
// #include "RadialGrad.cpp"


std::string GDrawSomething(GCanvas *canvas, GISize dim)
{
    canvas->clear({0, 0, 0, 1});
    GColor colors[] = {{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}};
    auto grad = GCreateLinearGradient({130, 130}, colors, 3, 40, GShader::TileMode::kRepeat);
    GPath path;
    path.addCircle({130, 130}, 40);
    // path.addRect(GRect::LTRB(0, 0, 255, 255));
    canvas->drawPath(path, GPaint(grad.get()));

    return "ur mommy";
}
