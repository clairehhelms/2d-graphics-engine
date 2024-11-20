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

template <typename DRAW> void spin(GCanvas* canvas, int N, DRAW draw) {
    for (int i = 0; i < N; ++i) {
        canvas->save();
        canvas->rotate(2 * M_PI * i / N);
        draw(canvas);
        canvas->restore();

    }
}

static void draw_cubic(GCanvas* canvas) {
    GPath path;
    path.moveTo(80 , 50).cubicTo(140, 250, 150, 50, 170, 50);
    GRandom rand;
    
    GColor limegreen = {.196, .804, .196, 1};

    canvas->translate(100, 150);
    // spin(canvas, 2, [&](GCanvas* canvas) {
        // canvas->drawPath(path, limegreen);
    // });
    canvas->drawPath(path, limegreen);

}

static void draw_cubics(GCanvas* canvas) {
    GPath path;
    path.moveTo(83 , 40).cubicTo(140, 250, 150, 50, 180, 40);
    GRandom rand;

    GPath path2;
    path2.moveTo(85, 45).cubicTo(130, 240, 140, 40, 170, 30);

    // path.addPolygon()
    GPath path3;
    GPoint pts[] = {
        {123, 130},
        {145, 220},
        {190, 230},
        {90, 230},
        {135, 220},
        {135, 110},
    };
    path3.moveTo(155, 164).addPolygon(pts, 6);
    // GMatrix mx;
    // mx.Translate(20, 10);
    // path3.transform(mx);
    
    auto rand_color = [&rand]() -> GColor {
        auto r = rand.nextF();
        auto g = rand.nextF();
        auto b = rand.nextF();
        return { r, g, b, 1 };

    };

    GPaint color1 = GPaint(rand_color());
    GPaint color2 = GPaint(rand_color());
    GPaint color3 = GPaint(rand_color());

    // canvas->translate(256, 256);
    canvas->drawPath(path, color1);
    canvas->drawPath(path3, color3);
    canvas->drawPath(path2, color2);
}

std::string GDrawSomething(GCanvas *canvas, GISize dim)
{
    canvas->clear({0, 0, 0, 1});
    // GPath path;
    // path.addCircle({0, 0}, 40, GPath::kCW_Direction);
    // // assert(false);
    // path.addCircle({0, 0}, 40 * .75f, GPath::kCCW_Direction);
    // canvas->translate(100, 100);

    // canvas->drawPath(path, GPaint({.888, .192, .388, 1}));
    draw_cubics(canvas);
    GColor colors[] = {{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}};
    // auto grad = GCreateLinearGradient({130, 130}, colors, 3, 40, GShader::TileMode::kRepeat);
    GPath path;
    path.addCircle({130, 130}, 40);
    // path.addRect(GRect::LTRB(0, 0, 255, 255));
    canvas->drawPath(path, colors[0]);


    return "picasso's goblet";
}
