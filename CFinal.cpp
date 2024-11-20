/*
 *  Copyright 2022 <Claire Helms>
 */

#include "GFinal.h"
#include "GPath.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <stack>
#include "RadialGrad.cpp"
#include "BilerpShader.cpp"

class CFinal : public GFinal
{
public:
    CFinal(){};

    /**
     *  Return a radial-gradient shader.
     *
     *  This is a shader defined by a circle with center and a radius.
     *  The array of colors are evenly distributed between the center (color[0]) out to
     *  the radius (color[count-1]). Beyond the radius, it respects the TileMode.
     */
    std::unique_ptr<GShader> createRadialGradient(GPoint center, float radius,
                                                  const GColor colors[], int count,
                                                  GShader::TileMode mode) override
    {
        return std::unique_ptr<GShader>(new RadialGrad(center, colors, count, radius, mode));
    }

    /**
     * Return a bitmap shader that performs kClamp tiling with a bilinear filter on each sample, respecting the
     * localMatrix.
     *
     * This is in contrast to the existing GCreateBitmapShader, which performs "nearest neightbor" sampling
     * when it fetches a pixel from the src bitmap.
     */
    std::unique_ptr<GShader> createBilerpShader(const GBitmap &bm,
                                                const GMatrix &localMatrix) override
    {
        return std::unique_ptr<GShader>(new BilerpShader(bm, localMatrix));
    }


    /**
     *  Add contour(s) to the specified path that will draw a line from p0 to p1 with the specified
     *  width and CapType. Note that "width" is the distance from one side of the stroke to the
     *  other, ala its thickness.
     */
    void addLine(GPath *path, GPoint p0, GPoint p1, float width, CapType cap) override
    {
        // find the perpendicular points to p0 and p1
        // then drawlines
        // and add an end on the enum
        float k = width / 2.0f;
        if (p0.fX > p1.fX)
        {
            std::swap(p0, p1);
        }
        // p0 is left, p1 is right
        float dx = p1.fX - p0.fX;
        float dy = p1.fY - p1.fY;
        GPoint v = {-dy, dx};
        v = v * (1.0f / sqrt(v.fX * v.fX + v.fY * v.fY));
        GPoint w = {dx, dy};
        w = w * (1.0f / sqrt(w.fX * w.fX + w.fY * w.fY));

        if (cap == CapType::kButt)
        {
            GPoint a = p0 + v * k;
            GPoint b = p0 - v * k;
            GPoint d = p1 + v * k;
            GPoint c = p1 - v * k;

            path->moveTo(a);
            path->lineTo(b);
            path->lineTo(c);
            path->lineTo(d);
        }
        else if (cap == CapType::kRound)
        {
            GPoint a = p0 + v * k;
            GPoint b = p0 - v * k;
            GPoint d = p1 + v * k;
            GPoint c = p1 - v * k;

            path->moveTo(a);
            path->lineTo(b);
            path->lineTo(c);
            path->lineTo(d);
            path->addCircle(p0, k, GPath::Direction::kCCW_Direction);
            path->addCircle(p1, k, GPath::Direction::kCCW_Direction);
        }
        else if (cap == CapType::kSquare)
        {
            GPoint a = p0 + v * k - w * k;
            GPoint b = p0 - v * k - w * k;
            GPoint d = p1 + v * k + w * k;
            GPoint c = p1 - v * k + w * k;

            path->moveTo(a);
            path->lineTo(b);
            path->lineTo(c);
            path->lineTo(d);
        }
    }

    /**
     * Add a rounded-rectangle to the path (which may already have other contours) in the specified winding direction.
     * Each corner should be a quarter-circular arc of the specified radius... if possible.
     * If the width and/or height of the rectangle cannot accomodate the radius, reduce the radius until it can be
     * accommodated.
     */
    void addRoundRect(GPath *path, const GRect &r, float radius, GPath::Direction dir) override {
        path->addRect(r, dir);
        // if (r.width() < r.height()) {
            // path->moveTo({r.left() + radius, r.top()});
            // path->quadTo({r.left(), r.top() + radius}, {r.left() + radius, r.bottom()});
            // path->lineTo({r.right() - radius, r.bottom()});
            // path->quadTo({r.right(), r.top() + radius}, {r.right() + radius, r.top()});
            // path->lineTo({r.left() + radius, r.top()});
        // }
        // else {
        //     path->moveTo({r.left() + radius, r.top()});
        //     path->quadTo({r.left(), r.top() + radius}, {r.left() + radius, r.bottom()});
        //     path->lineTo({r.right() - radius, r.bottom()});
        //     path->quadTo({r.right(), r.top() + radius}, {r.right() + radius, r.top()});
        //     path->lineTo({r.left() + radius, r.top()});
        // }

    }
};

/**
 *  Implement this to return ain instance of your subclass of GFinal.
 */
std::unique_ptr<GFinal> GCreateFinal()
{
    return std::unique_ptr<GFinal>(new CFinal());
};
