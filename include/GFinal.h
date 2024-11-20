/*
 *  Copyright 2020 Mike Reed
 */

#ifndef GFinal_DEFINED
#define GFinal_DEFINED

#include "GCanvas.h"
#include "GPath.h"
#include "GShader.h"

/**
 * Override and implement these methods. You must implement GCreateFinal() to return your subclass.
 *
 * Each method has a default (do nothing) impl, so you only need to override the ones you intended to implement.
 */
class GFinal {
public:
    virtual ~GFinal() {}

    /**
     *  Return a radial-gradient shader.
     *
     *  This is a shader defined by a circle with center and a radius.
     *  The array of colors are evenly distributed between the center (color[0]) out to
     *  the radius (color[count-1]). Beyond the radius, it respects the TileMode.
     */
    virtual std::unique_ptr<GShader> createRadialGradient(GPoint center, float radius,
                                                          const GColor colors[], int count,
                                                          GShader::TileMode mode) {
        return nullptr;
    }

    /**
     * Return a bitmap shader that performs kClamp tiling with a bilinear filter on each sample, respecting the
     * localMatrix.
     *
     * This is in contrast to the existing GCreateBitmapShader, which performs "nearest neightbor" sampling
     * when it fetches a pixel from the src bitmap.
     */
    virtual std::unique_ptr<GShader> createBilerpShader(const GBitmap&,
                                                        const GMatrix& localMatrix) {
        return nullptr;
    }

    enum CapType {
        kButt,      // no cap on the line
        kSquare,    // square cap extending width/2
        kRound,     // round cap with radius = width/2
    };

    /**
     *  Add contour(s) to the specified path that will draw a line from p0 to p1 with the specified
     *  width and CapType. Note that "width" is the distance from one side of the stroke to the
     *  other, ala its thickness.
     */
    virtual void addLine(GPath* path, GPoint p0, GPoint p1, float width, CapType) {}

    /**
     * Add a rounded-rectangle to the path (which may already have other contours) in the specified winding direction.
     * Each corner should be a quarter-circular arc of the specified radius... if possible.
     * If the width and/or height of the rectangle cannot accomodate the radius, reduce the radius until it can be
     * accommodated.
     */
    virtual void addRoundRect(GPath* path, const GRect& r, float radius, GPath::Direction) {}
};

/**
 *  Implement this to return ain instance of your subclass of GFinal.
 */
std::unique_ptr<GFinal> GCreateFinal();

#endif
